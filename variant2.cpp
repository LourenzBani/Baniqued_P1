// variant2.cpp: Divisibility testing with threads, print immediately
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <string>
#include <atomic>
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

bool is_prime_threaded(long n, int num_threads) {
    if (n <= 1) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    long sqrt_n = static_cast<long>(sqrt(n)) + 1;
    atomic<bool> has_divisor(false);
    vector<thread> threads;
    long range_start = 5;
    long range_size = (sqrt_n - range_start) / num_threads + 1;
    int active_threads = 0;
    for (int i = 0; i < num_threads; ++i) {
        long thread_start = range_start + i * range_size;
        long thread_end = min(thread_start + range_size, sqrt_n + 1);
        if (thread_start >= thread_end) break;
        active_threads++;
        threads.emplace_back([n, thread_start, thread_end, &has_divisor]() {
            for (long d = thread_start; d < thread_end; d += 2) {
                if (has_divisor.load()) return;
                if (n % d == 0) {
                    has_divisor.store(true);
                    return;
                }
            }
        });
    }
    for (auto& t : threads) t.join();
    return !has_divisor.load();
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

    for (long n = 2; n <= limit; ++n) {
        if (is_prime_threaded(n, num_threads)) {
            lock_guard<mutex> lock(print_mutex);
            cout << "Thread main at " << get_timestamp() << " found " << n << endl;
        }
    }

    {
        lock_guard<mutex> lock(print_mutex);
        cout << "End: " << get_timestamp() << endl;
    }

    return 0;
}