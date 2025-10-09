// variant1.cpp: Straight division, print immediately
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <string>
#include <sstream>

using namespace std;

mutex print_mutex;

string get_timestamp() {
    auto now = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(now);
    tm* ptm = localtime(&tt);
    stringstream ss;
    ss << put_time(ptm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

bool is_prime(long n) {
    if (n <= 1) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

int main() {
    ifstream config("config.txt");
    int num_threads, limit_int;
    config >> num_threads >> limit_int;
    long limit = limit_int;

    {
        lock_guard<mutex> lock(print_mutex);
        cout << "Start: " << get_timestamp() << endl;
    }

    vector<thread> threads;
    long chunk = (limit - 1) / num_threads;
    for (int i = 0; i < num_threads; ++i) {
        long start = 2 + i * chunk;
        long end = (i == num_threads - 1) ? limit + 1 : start + chunk;
        threads.emplace_back([i, start, end]() {
            for (long num = start; num < end; ++num) {
                if (is_prime(num)) {
                    lock_guard<mutex> lock(print_mutex);
                    cout << "Thread " << i << " at " << get_timestamp() << " found " << num << endl;
                }
            }
        });
    }

    for (auto& t : threads) t.join();

    {
        lock_guard<mutex> lock(print_mutex);
        cout << "End: " << get_timestamp() << endl;
    }

    return 0;
}