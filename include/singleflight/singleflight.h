#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace singleflight {

class FuncCallFailedException : public std::runtime_error {
   public:
    using std::runtime_error::runtime_error;

    FuncCallFailedException(const FuncCallFailedException&) = default;
    FuncCallFailedException& operator=(const FuncCallFailedException&) = default;
    FuncCallFailedException(FuncCallFailedException&&) = default;
    FuncCallFailedException& operator=(FuncCallFailedException&&) = default;

    ~FuncCallFailedException() noexcept override;
};

FuncCallFailedException::~FuncCallFailedException() noexcept {}

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
            // Check exception and result
            if (ptr->has_ex) {
                throw ptr->ex;
            }
            return ptr->result;
        }

        // The current thread is responsible for fetching the result
        auto ptr = std::make_shared<DoingRecord>();
        // Tell other threads the result is being fetched
        _doing[key] = ptr;
        lock.unlock();

        try {
            // Call the actual function to fetch the result
            ptr->result = func(std::forward<Args>(args)...);
        } catch (const std::exception& ex) {
            ptr->has_ex = true;
            ptr->ex = FuncCallFailedException(ex.what());
        } catch (...) {
            ptr->has_ex = true;
            ptr->ex = FuncCallFailedException("Func call threw non-std-exception");
        }

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
        if (ptr->has_ex) {
            throw ptr->ex;
        }
        return ptr->result;
    }

   private:
    struct DoingRecord {
        bool done;
        std::mutex mtx;
        std::condition_variable cv;
        TRet result;
        bool has_ex;
        FuncCallFailedException ex;

        DoingRecord() : done(false), has_ex(false), ex("") {}
    };
    std::unordered_map<TKey, std::shared_ptr<DoingRecord>> _doing;
    std::mutex _doing_mtx;
};

}  // namespace singleflight
