#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace singleflight {

template <typename TKey, typename TRet>
class SingleFlight {
   public:
    template <class F, typename... Args>
    TRet Do(const TKey& key, F&& func, Args&&... args) {
        // Acquire the lock
        std::unique_lock<std::mutex> lock(_doing_mtx);

        auto it = _doing.find(key);
        // An another thread has been fetching the result
        if (it != _doing.end()) {
            lock.unlock();
            // Just waiting for the result
            auto ptr = it->second;
            std::unique_lock<std::mutex> done_lock(ptr->mtx);
            ptr->cv.wait(done_lock, [ptr]() -> bool { return ptr->done; });
            // Directly return the result
            return ptr->result;
        }

        // The current thread is responsible for fetching the result
        auto ptr = std::make_shared<DoingRecord>();
        // Tell other threads the result is being fetched
        _doing[key] = ptr;
        lock.unlock();

        // Call the actual function to fetch the result
        ptr->result = func(args...);
        {
            std::lock_guard<std::mutex> done_lock_guard(ptr->mtx);
            ptr->done = true;
        }

        // Notify other threads to read the result
        ptr->cv.notify_all();
        // Remove record from "doing"
        lock.lock();
        _doing.erase(key);

        // Return the result for the current thread
        return ptr->result;
    }

   private:
    struct DoingRecord {
        bool done;
        std::mutex mtx;
        std::condition_variable cv;
        TRet result;

        DoingRecord() : done(false) {}
    };
    std::unordered_map<TKey, std::shared_ptr<DoingRecord>> _doing;
    std::mutex _doing_mtx;
};

}  // namespace singleflight
