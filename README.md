[![codecov](https://codecov.io/gh/hongshibao/cpp-singleflight/branch/main/graph/badge.svg?token=SW62SVNATO)](https://codecov.io/gh/hongshibao/cpp-singleflight)

# cpp-singleflight

A header-only/compiled C++ singleflight library.

The singleflight library is to suppress duplicate function calls into one, which is useful in preventing [thundering-herd](https://en.wikipedia.org/wiki/Thundering_herd_problem)/[cache-stampede](https://en.wikipedia.org/wiki/Cache_stampede) to database or other backends. This is a C++ port of [Golang's singleflight implementation](https://github.com/golang/sync/blob/master/singleflight/singleflight.go).

## Install

### Header-only version

Copy the include [folder](./include) to your build tree and use a C++14 compiler.

### Compiled version

```bash
git clone https://github.com/hongshibao/cpp-singleflight
cd cpp-singleflight && mkdir build && cd build
cmake .. && make -j
```

Then you can also run `make install` to install the library. cmake option `CMAKE_INSTALL_PREFIX` can be set to change the installation folder:
```bash
cmake -DCMAKE_INSTALL_PREFIX=/the/path/to/install ..
```

There are some other available cmake options for this project, such as:

1. `SINGLEFLIGHT_BUILD_SHARED_LIB`: to build shared lib or static lib.
2. `SINGLEFLIGHT_BUILD_TESTS`: whether to build unit tests.
3. `SINGLEFLIGHT_BUILD_EXAMPLES`: whether to build examples.

## Usage Samples

The examples code is in [examples/example.cpp](./examples/example.cpp).

### Singleflight basic example

```c++
#include <singleflight/singleflight.h>
#include <spdlog/spdlog.h>

#include <string>
#include <thread>
#include <vector>

using namespace std;

// SingleFlight example code
void example_1() {
    singleflight::SingleFlight<string, int> sf;

    // Simulate a heavy function call
    auto long_running_func = [](int tid) -> int {
        spdlog::info("long_running_func call by Thread {}", tid);
        this_thread::sleep_for(1000ms);
        return 100;
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
```

### Singleflight example with exception thrown

The singleflight `Do` will throw `FuncCallFailedException` if the actual function call throws exception:

```c++
#include <singleflight/singleflight.h>
#include <spdlog/spdlog.h>

#include <string>
#include <thread>
#include <vector>

using namespace std;

// SingleFlight example code (with std::exception thrown)
void example_2() {
    singleflight::SingleFlight<string, int> sf;

    // Simulate a function call which throws std::exception
    auto throwing_exception_func = [](int tid) -> int {
        spdlog::info("throwing_exception_func call by Thread {}", tid);
        this_thread::sleep_for(500ms);
        throw runtime_error{"std::runtime_error from throwing_exception_func"};
        return 200;
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
```
