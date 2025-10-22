#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iomanip>

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

std::mutex output_lock;

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

void find_primes_in_segment(uint64_t lower, uint64_t upper, int thread_id) {
    auto begin_time = std::chrono::system_clock::now();
    
    {
        std::lock_guard<std::mutex> guard(output_lock);
        std::cout << "[Thread " << thread_id << "] Starting range " << lower << "-" << upper 
                  << " at " << get_timestamp(begin_time) << std::endl;
    }

    for (uint64_t candidate = lower; candidate <= upper; ++candidate) {
        if (check_primality(candidate)) {
            auto now = std::chrono::system_clock::now();
            std::lock_guard<std::mutex> guard(output_lock);
            std::cout << "[Thread " << thread_id << "] Found prime: " << candidate 
                      << " (Time: " << get_timestamp(now) << ")" << std::endl;
        }
    }

    auto end_time = std::chrono::system_clock::now();
    std::lock_guard<std::mutex> guard(output_lock);
    std::cout << "[Thread " << thread_id << "] Completed at " << get_timestamp(end_time) << std::endl;
}

void execute_prime_search(const Settings& cfg) {
    std::cout << "\n========== VARIANT A1-B1 ==========" << std::endl;
    std::cout << "A1: Print Immediately | B1: Straight Division of Search Range" << std::endl;
    std::cout << "Configuration: " << cfg.thread_count << " threads, searching up to " << cfg.upper_limit << std::endl;

    auto program_start = std::chrono::system_clock::now();
    std::cout << "Start Time: " << get_timestamp(program_start) << "\n" << std::endl;

    std::vector<std::thread> workers;
    uint64_t segment_size = cfg.upper_limit / cfg.thread_count;

    for (int i = 0; i < cfg.thread_count; ++i) {
        uint64_t lower = (i == 0) ? 2 : (i * segment_size + 1);
        uint64_t upper = (i == cfg.thread_count - 1) ? cfg.upper_limit : ((i + 1) * segment_size);
        workers.emplace_back(find_primes_in_segment, lower, upper, i);
    }

    for (auto& worker : workers) worker.join();

    auto program_end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(program_end - program_start);

    std::cout << "\n=================================================================" << std::endl;
    std::cout << "End Time: " << get_timestamp(program_end) << std::endl;
    std::cout << "Execution Time: " << elapsed.count() << " ms" << std::endl;
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
