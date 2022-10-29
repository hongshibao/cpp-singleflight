#include <singleflight/singleflight.h>
#include <spdlog/spdlog.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

// SingleFlight example code 1
void example_1(singleflight::SingleFlight<string, string>& sf) {
    // Simulate a heavy function call
    auto long_running_func = [](int tid) -> string {
        spdlog::info("long_running_func call by Thread {}", tid);
        this_thread::sleep_for(1000ms);
        return "Result from long_running_func";
    };

    // Thread entry function
    auto thread_entry_func = [&](int tid) {
        spdlog::info("Thread {} starts", tid);
        auto res = sf.Do("some-key", long_running_func, tid);
        spdlog::info("Thread {} result: {}", tid, res);
    };

    // Launch threads
    constexpr int THREADS_NUM = 5;
    vector<shared_ptr<thread>> threads;
    for (int i = 0; i < THREADS_NUM; ++i) {
        threads.push_back(make_shared<thread>(thread_entry_func, i));
    }
    // Waiting
    for (auto t : threads) {
        t->join();
    }
}

// SingleFlight example code 2 (with std::exception thrown)
void example_2(singleflight::SingleFlight<string, string>& sf) {
    // Simulate a function call which throws std::exception
    auto throwing_exception_func = [](int tid) -> string {
        spdlog::info("throwing_exception_func call by Thread {}", tid);
        this_thread::sleep_for(500ms);
        throw runtime_error{"std::runtime_error from throwing_exception_func"};
        return "Result from throwing_exception_func";
    };

    // Thread entry function
    auto thread_entry_func = [&](int tid) {
        spdlog::info("Thread {} starts", tid);
        try {
            auto res = sf.Do("some-key", throwing_exception_func, tid);
            spdlog::info("Thread {} result: {}", tid, res);
        } catch (const singleflight::FuncCallFailedException& ex) {
            spdlog::info("Caught exception in Thread {}: {}", tid, ex.what());
            return;
        }
    };

    // Launch threads
    constexpr int THREADS_NUM = 5;
    vector<shared_ptr<thread>> threads;
    for (int i = 0; i < THREADS_NUM; ++i) {
        threads.push_back(make_shared<thread>(thread_entry_func, i));
    }
    // Waiting
    for (auto t : threads) {
        t->join();
    }
}

int main() {
    singleflight::SingleFlight<string, string> sf;

    // Run example_1
    spdlog::info("====== Running example_1 ======");
    example_1(sf);
    spdlog::info("====== Finished example_1 ======\n");

    // Run example_2
    spdlog::info("====== Running example_2 ======");
    example_2(sf);
    spdlog::info("====== Finished example_2 ======\n");

    return 0;
}
