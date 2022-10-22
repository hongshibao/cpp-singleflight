#include <singleflight/singleflight.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

// SingleFlight example code
void example_1(singleflight::SingleFlight<string, string>& sf) {
    // Simulate a heavy function call
    auto long_running_func = [](int tid) -> string {
        cout << "long_running_func call by Thread " << tid << endl;
        this_thread::sleep_for(1000ms);
        return "Result from long_running_func";
    };

    // Thread entry function
    auto thread_entry_func = [&](int tid) {
        cout << "Thread " << tid << " starts" << endl;
        auto res = sf.Do("some-key", long_running_func, tid);
        cout << "Thread " << tid << " result: " << res << endl;
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

// SingleFlight example code (with std::exception thrown)
void example_2(singleflight::SingleFlight<string, string>& sf) {
    // Simulate a function call which throws std::exception
    auto throwing_exception_func = [](int tid) -> string {
        cout << "throwing_exception_func call by Thread " << tid << endl;
        this_thread::sleep_for(500ms);
        throw runtime_error{"std::runtime_error from throwing_exception_func"};
        return "Result from throwing_exception_func";
    };

    // Thread entry function
    auto thread_entry_func = [&](int tid) {
        cout << "Thread " << tid << " starts" << endl;
        try {
            auto res = sf.Do("some-key", throwing_exception_func, tid);
            cout << "Thread " << tid << " result: " << res << endl;
        } catch (const singleflight::FuncCallFailedException& ex) {
            cout << "Caught exception in Thread " << tid << ": " << ex.what() << endl;
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
    cout << "====== Running example_1 ======" << endl;
    example_1(sf);
    cout << "====== Finished example_1 ======" << endl
         << endl;

    // Run example_2
    cout << "====== Running example_2 ======" << endl;
    example_2(sf);
    cout << "====== Finished example_2 ======" << endl
         << endl;

    return 0;
}
