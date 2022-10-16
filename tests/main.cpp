#include <singleflight/singleflight.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

int main() {
    singleflight::SingleFlight<string, vector<int>> sf;

    auto long_running_f = [](int tid) -> vector<int> {
        cout << "Long running call by Thread " << tid << endl;
        this_thread::sleep_for(500ms);
        return {1, 2, 3};
    };

    auto f = [&](int tid) -> vector<int> {
        cout << "Thread " << tid << " starts" << endl;
        auto res = sf.Do("a-key", long_running_f, tid);
        cout << "Thread " << tid << " result: ";
        for (auto it : res) {
            cout << " " << it;
        }
        cout << endl;
        return res;
    };

    constexpr int THREADS_NUM = 5;
    vector<shared_ptr<thread>> threads;
    for (int i = 0; i < THREADS_NUM; ++i) {
        threads.push_back(make_shared<thread>(f, i));
    }
    for (auto t : threads) {
        t->join();
    }
    return 0;
}
