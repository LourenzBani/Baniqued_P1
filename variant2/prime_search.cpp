#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <atomic>

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

std::string get_timestamp(const std::chrono::system_clock::time_point& point) {
    auto tt = std::chrono::system_clock::to_time_t(point);
    std::stringstream stream;
    stream << std::put_time(std::localtime(&tt), "%H:%M:%S");
    return stream.str();
}

// Thread-based divisibility testing with logging
bool check_primality_threaded(uint64_t num, int num_threads) {
    if (num < 2) return false;
    if (num == 2) return true;
    
    // Check divisibility by 2
    if (num % 2 == 0) {
        auto now = std::chrono::system_clock::now();
        std::lock_guard<std::mutex> guard(output_lock);
        std::cout << "[" << get_timestamp(now) << "] [Thread 0] checked divisor 2 for " << num 
                  << " - COMPOSITE" << std::endl;
        return false;
    }
    
    if (num == 3) return true;
    
    uint64_t sqrt_n = static_cast<uint64_t>(std::sqrt(num));
    if (sqrt_n < 3) return true;
    
    std::atomic<bool> is_composite(false);
    std::vector<std::thread> workers;
    
    // Divide the range of potential divisors among threads
    uint64_t range_size = (sqrt_n - 3) / 2 + 1;  // odd numbers from 3 to sqrt_n
    uint64_t chunk = (range_size + num_threads - 1) / num_threads;
    
    for (int i = 0; i < num_threads; ++i) {
        workers.emplace_back([&, i, num, sqrt_n, chunk]() {
            uint64_t start = 3 + i * chunk * 2;
            uint64_t end = std::min(start + chunk * 2, sqrt_n + 1);
            
            for (uint64_t div = start; div < end && !is_composite.load(); div += 2) {
                auto now = std::chrono::system_clock::now();
                {
                    std::lock_guard<std::mutex> guard(output_lock);
                    std::cout << "[" << get_timestamp(now) << "] [Thread " << i 
                              << "] checking divisor " << div << " for " << num << std::endl;
                }
                
                if (num % div == 0) {
                    is_composite.store(true);
                    auto now2 = std::chrono::system_clock::now();
                    std::lock_guard<std::mutex> guard(output_lock);
                    std::cout << "[" << get_timestamp(now2) << "] [Thread " << i 
                              << "] divisor " << div << " divides " << num 
                              << " - COMPOSITE" << std::endl;
                    return;
                }
            }
        });
    }
    
    for (auto& w : workers) w.join();
    
    return !is_composite.load();
}

std::atomic<uint64_t> total_numbers_processed(0);
std::atomic<uint64_t> total_primes_found(0);

void execute_prime_search(const Settings& cfg) {
    std::cout << "\n========== VARIANT A1-B2 ==========" << std::endl;
    std::cout << "A1: Print Immediately | B2: Threads for Divisibility Testing" << std::endl;
    std::cout << "Configuration: " << cfg.thread_count << " threads for divisibility testing | Upper Limit: " << cfg.upper_limit << std::endl;

    auto program_start = std::chrono::system_clock::now();
    std::cout << "Program Start: " << get_timestamp(program_start) << "\n" << std::endl;

    total_numbers_processed.store(0);
    total_primes_found.store(0);

    // Linear search through numbers, using threads for divisibility testing
    for (uint64_t num = 2; num <= cfg.upper_limit; ++num) {
        total_numbers_processed++;
        
        if (check_primality_threaded(num, cfg.thread_count)) {
            total_primes_found++;
            auto now = std::chrono::system_clock::now();
            std::lock_guard<std::mutex> guard(output_lock);
            std::cout << "[" << get_timestamp(now) << "] [Main Thread] Prime found: " << num << std::endl;
        }
    }

    auto program_end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(program_end - program_start);

    std::cout << "\n=======================================================================" << std::endl;
    std::cout << "Program End: " << get_timestamp(program_end) << std::endl;
    std::cout << "Numbers Processed: " << total_numbers_processed.load() << std::endl;
    std::cout << "Total Primes Found: " << total_primes_found.load() << std::endl;
    std::cout << "Total Execution Time: " << elapsed.count() << " ms" << std::endl;
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
