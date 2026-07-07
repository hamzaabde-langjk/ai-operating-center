#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include <csignal>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <dirent.h>

std::atomic<bool> g_running{true};

void signalHandler(int) {
    g_running = false;
}

struct SystemStats {
    double cpuUsage{0.0};
    double memUsage{0.0};
    double memTotal{0.0};
    double memFree{0.0};
    uint64_t netRx{0};
    uint64_t netTx{0};
    uint64_t diskRead{0};
    uint64_t diskWrite{0};
    uint32_t processCount{0};
    uint32_t threadCount{0};
};

SystemStats readSystemStats() {
    SystemStats stats;

    std::ifstream statFile("/proc/stat");
    if (statFile.is_open()) {
        std::string line;
        std::getline(statFile, line);
        std::istringstream iss(line);
        std::string cpu;
        unsigned long user, nice, system, idle, iowait, irq, softirq, steal;
        iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
        unsigned long total = user + nice + system + idle + iowait + irq + softirq + steal;
        unsigned long active = total - idle - iowait;
        stats.cpuUsage = total > 0 ? (100.0 * active / total) : 0.0;
    }

    std::ifstream memFile("/proc/meminfo");
    if (memFile.is_open()) {
        std::string line;
        while (std::getline(memFile, line)) {
            std::istringstream iss(line);
            std::string key;
            unsigned long value;
            std::string unit;
            iss >> key >> value >> unit;

            if (key == "MemTotal:") stats.memTotal = value / 1024.0;
            else if (key == "MemAvailable:") stats.memFree = value / 1024.0;
        }
        stats.memUsage = stats.memTotal > 0 ? 
            (100.0 * (stats.memTotal - stats.memFree) / stats.memTotal) : 0.0;
    }

    std::ifstream netFile("/proc/net/dev");
    if (netFile.is_open()) {
        std::string line;
        std::getline(netFile, line);
        std::getline(netFile, line);
        while (std::getline(netFile, line)) {
            std::istringstream iss(line);
            std::string iface;
            unsigned long rx, tx;
            iss >> iface >> rx;
            for (int i = 0; i < 7; ++i) {
                unsigned long skip;
                iss >> skip;
            }
            iss >> tx;

            if (iface != "lo:") {
                stats.netRx += rx;
                stats.netTx += tx;
            }
        }
    }

    DIR* dir = opendir("/proc");
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (entry->d_type == DT_DIR) {
                char* endptr;
                strtoul(entry->d_name, &endptr, 10);
                if (*endptr == '\0') stats.processCount++;
            }
        }
        closedir(dir);
    }

    return stats;
}

void printStats(const SystemStats& stats) {
    std::cout << "\033[2J\033[H";
    std::cout << "Linux X-Ray Vision - System Monitor" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "CPU Usage:    " << std::fixed << std::setprecision(1) 
              << stats.cpuUsage << "%" << std::endl;
    std::cout << "Memory:       " << std::fixed << std::setprecision(1)
              << (stats.memTotal - stats.memFree) << " / " << stats.memTotal 
              << " MB (" << stats.memUsage << "%)" << std::endl;
    std::cout << "Network RX:   " << stats.netRx << " bytes" << std::endl;
    std::cout << "Network TX:   " << stats.netTx << " bytes" << std::endl;
    std::cout << "Processes:    " << stats.processCount << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Press Ctrl+C to stop monitoring" << std::endl;
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    spdlog::info("xray-monitor started");

    int interval = 1;
    if (argc > 1) {
        interval = std::atoi(argv[1]);
        if (interval < 1) interval = 1;
    }

    while (g_running) {
        auto stats = readSystemStats();
        printStats(stats);

        for (int i = 0; i < interval && g_running; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    spdlog::info("xray-monitor stopped");
    return 0;
}
