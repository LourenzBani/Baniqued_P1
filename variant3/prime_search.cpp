#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>

std::string strip_whitespace(const std::string& str) {
    auto start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    auto end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

struct Settings {
    int thread_count;
    uint64_t upper_limit;
};

struct PrimeData {
    uint64_t value;
    std::chrono::system_clock::time_point discovered_at;
    int worker_id;
};

std::mutex output_lock;
std::mutex data_lock;
std::vector<PrimeData> discovered_primes;

Settings load_configuration(const std::string& filepath) {
    Settings settings;
    std::ifstream input(filepath);
    if (!input.is_open()) {
        throw std::runtime_error("Failed to open configuration file: " + filepath);
    }

    std::string line;
    while (std::getline(input, line)) {
        line = strip_whitespace(line);
        if (line.empty() || line[0] == '#') continue;

        auto delimiter = line.find('=');
        if (delimiter == std::string::npos) continue;

        std::string key = strip_whitespace(line.substr(0, delimiter));
        std::string val = strip_whitespace(line.substr(delimiter + 1));

        if (key == "Threads") {
            settings.thread_count = std::stoi(val);
        } else if (key == "Max Value") {
            if (val.find("2^") == 0) {
                int exp = std::stoi(val.substr(2));
                settings.upper_limit = (exp == 64) ? UINT64_MAX : (1ULL << exp);
            } else {
                settings.upper_limit = std::stoull(val);
            }
        }
    }
    return settings;
}

inline bool check_primality(uint64_t num) {
    if (num < 2) return false;
    if (num == 2) return true;
    if (num % 2 == 0) return false;
    uint64_t limit = static_cast<uint64_t>(std::sqrt(num));
    for (uint64_t i = 3; i <= limit; i += 2) {
        if (num % i == 0) return false;
    }
    return true;
}

std::string get_timestamp(const std::chrono::system_clock::time_point& point) {
    auto tt = std::chrono::system_clock::to_time_t(point);
    std::stringstream stream;
    stream << std::put_time(std::localtime(&tt), "%H:%M:%S");
    return stream.str();
}

void collect_primes_from_range(uint64_t lower, uint64_t upper, int thread_id) {
    auto begin_time = std::chrono::system_clock::now();
    
    {
        std::lock_guard<std::mutex> guard(output_lock);
        std::cout << "Thread " << thread_id << " started: range [" << lower << "-" << upper 
                  << "] @ " << get_timestamp(begin_time) << std::endl;
    }

    std::vector<PrimeData> local_primes;

    for (uint64_t candidate = lower; candidate <= upper; ++candidate) {
        if (check_primality(candidate)) {
            local_primes.push_back({candidate, std::chrono::system_clock::now(), thread_id});
        }
    }

    {
        std::lock_guard<std::mutex> guard(data_lock);
        discovered_primes.insert(discovered_primes.end(), local_primes.begin(), local_primes.end());
    }

    auto end_time = std::chrono::system_clock::now();
    std::lock_guard<std::mutex> guard(output_lock);
    std::cout << "Thread " << thread_id << " completed @ " << get_timestamp(end_time) 
              << " (Found " << local_primes.size() << " primes)" << std::endl;
}

void execute_prime_search(const Settings& cfg) {
    std::cout << "\n========== VARIANT A2-B1 ==========" << std::endl;
    std::cout << "A2: Wait Then Print Everything | B1: Straight Division of Search Range" << std::endl;
    std::cout << "Configuration: " << cfg.thread_count << " threads | Max: " << cfg.upper_limit << std::endl;

    discovered_primes.clear();
    auto program_start = std::chrono::system_clock::now();
    std::cout << "Start: " << get_timestamp(program_start) << "\n" << std::endl;

    std::vector<std::thread> workers;
    uint64_t segment_size = cfg.upper_limit / cfg.thread_count;

    for (int i = 0; i < cfg.thread_count; ++i) {
        uint64_t lower = (i == 0) ? 2 : (i * segment_size + 1);
        uint64_t upper = (i == cfg.thread_count - 1) ? cfg.upper_limit : ((i + 1) * segment_size);
        workers.emplace_back(collect_primes_from_range, lower, upper, i);
    }

    for (auto& worker : workers) worker.join();

    std::cout << "\n--- Results (sorted by value) ---" << std::endl;
    std::cout << "Total Primes Found: " << discovered_primes.size() << "\n" << std::endl;
    
    std::sort(discovered_primes.begin(), discovered_primes.end(), 
              [](const PrimeData& a, const PrimeData& b) { return a.value < b.value; });

    for (const auto& prime : discovered_primes) {
        std::cout << "[T" << prime.worker_id << "] Prime: " << prime.value 
                  << " | Found at: " << get_timestamp(prime.discovered_at) << std::endl;
    }

    // Summary by thread
    std::cout << "\n=== Summary by Thread ===" << std::endl;
    std::map<int, std::vector<uint64_t>> thread_primes;
    for (const auto& p : discovered_primes) {
        thread_primes[p.worker_id].push_back(p.value);
    }
    
    for (const auto& [thread_id, primes] : thread_primes) {
        std::cout << "Thread " << thread_id << " found " << primes.size() << " primes: ";
        int show_count = std::min(5, static_cast<int>(primes.size()));
        for (int i = 0; i < show_count; ++i) {
            std::cout << primes[i];
            if (i < show_count - 1) std::cout << ", ";
        }
        if (primes.size() > 5) {
            std::cout << ", ... and " << (primes.size() - 5) << " more";
        }
        std::cout << std::endl;
    }

    auto program_end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(program_end - program_start);

    std::cout << "\n=============================================================" << std::endl;
    std::cout << "End: " << get_timestamp(program_end) << std::endl;
    std::cout << "Runtime: " << elapsed.count() << " ms" << std::endl;
}

int main() {
    try {
        Settings cfg = load_configuration("config.txt");
        std::cout << "\n[Configuration Loaded]" << std::endl;
        std::cout << "Thread Count: " << cfg.thread_count << std::endl;
        std::cout << "Upper Limit: " << cfg.upper_limit << std::endl;
        execute_prime_search(cfg);
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
