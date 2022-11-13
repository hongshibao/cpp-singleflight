#include <singleflight/singleflight.h>
#include <spdlog/spdlog.h>

#include <atomic>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace singleflight;
using Catch::Matchers::Message;

namespace {

template <typename ExType>
string ThrowingExceptionFunc(int tid, atomic_int& func_call_cnt, const ExType& ex) {
    ++func_call_cnt;
    spdlog::info("throwing_exception_func call by Thread {}", tid);
    this_thread::sleep_for(500ms);
    throw ex;
    return "Result from throwing_exception_func";
}

void LaunchAndWaitThreads(function<void(int)>&& thread_entry_func) {
    // Launch threads
    static constexpr int THREADS_NUM = 5;
    vector<shared_ptr<thread>> threads;
    for (int i = 0; i < THREADS_NUM; ++i) {
        threads.push_back(make_shared<thread>(thread_entry_func, i));
    }
    // Waiting
    for (auto t : threads) {
        t->join();
    }
}

}  // namespace

TEST_CASE("Multiple threads call a same func using a same key") {
    SingleFlight<string, string> sf;
    atomic_int func_call_cnt{0};

    // Simulate a heavy function call
    const string LONG_RUNNING_FUNC_RESULT = "Result from long_running_func";
    auto long_running_func = [&](int tid) -> string {
        ++func_call_cnt;
        spdlog::info("long_running_func call by Thread {}", tid);
        this_thread::sleep_for(1000ms);
        return LONG_RUNNING_FUNC_RESULT;
    };

    // Thread entry function
    auto thread_entry_func = [&](int tid) {
        spdlog::info("Thread {} starts", tid);
        auto res = sf.Do("some-key", long_running_func, tid);
        spdlog::info("Thread {} result: {}", tid, res);
        REQUIRE(res == LONG_RUNNING_FUNC_RESULT);
    };

    // Launch threads and wait
    LaunchAndWaitThreads(thread_entry_func);

    // long_running_func should only be called once
    REQUIRE(func_call_cnt.load() == 1);
}

TEST_CASE("Multiple threads call a same func which throws std::exception") {
    SingleFlight<string, string> sf;
    atomic_int func_call_cnt{0};

    // Simulate a function call which throws std::exception
    auto throwing_exception_func = [&](int tid) -> string {
        return ThrowingExceptionFunc<runtime_error>(
            tid,
            func_call_cnt,
            runtime_error{"std::runtime_error from throwing_exception_func"});
    };

    // Thread entry function
    auto thread_entry_func = [&](int tid) {
        spdlog::info("Thread {} starts", tid);
        REQUIRE_THROWS_AS(sf.Do("some-key", throwing_exception_func, tid), FuncCallFailedException);
    };

    // Launch threads and wait
    LaunchAndWaitThreads(thread_entry_func);

    // throwing_exception_func should only be called once
    REQUIRE(func_call_cnt.load() == 1);
}

TEST_CASE("Multiple threads call a same func which throws non std::exception") {
    SingleFlight<string, string> sf;
    atomic_int func_call_cnt{0};

    // Simulate a function call which throws non std::exception
    auto throwing_exception_func = [&](int tid) -> string {
        return ThrowingExceptionFunc<string>(
            tid,
            func_call_cnt,
            "non std::exception from throwing_exception_func");
    };

    // Thread entry function
    auto thread_entry_func = [&](int tid) {
        spdlog::info("Thread {} starts", tid);
        REQUIRE_THROWS_MATCHES(sf.Do("some-key", throwing_exception_func, tid),
                               FuncCallFailedException,
                               Message("Func call threw non-std-exception"));
    };

    // Launch threads and wait
    LaunchAndWaitThreads(thread_entry_func);

    // throwing_exception_func should only be called once
    REQUIRE(func_call_cnt.load() == 1);
}
