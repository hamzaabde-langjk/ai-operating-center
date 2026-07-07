#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <spdlog/spdlog.h>

struct ProcessInfo {
    uint32_t pid;
    uint32_t ppid;
    std::string comm;
    double cpuPercent;
    double memPercent;
    uint64_t rss;
    uint32_t threads;
    std::string state;
};

std::vector<ProcessInfo> readProcesses() {
    std::vector<ProcessInfo> processes;

    DIR* dir = opendir("/proc");
    if (!dir) return processes;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type != DT_DIR) continue;

        char* endptr;
        uint32_t pid = static_cast<uint32_t>(strtoul(entry->d_name, &endptr, 10));
        if (*endptr != '\0') continue;

        ProcessInfo info{pid, 0, "", 0.0, 0.0, 0, 0, ""};

        std::string statPath = std::string("/proc/") + entry->d_name + "/stat";
        std::ifstream statFile(statPath);
        if (statFile.is_open()) {
            std::string line;
            std::getline(statFile, line);
            std::istringstream iss(line);

            uint32_t p;
            std::string comm;
            char state;

            iss >> p >> comm >> state >> info.ppid;
            info.state = std::string(1, state);

            for (int i = 0; i < 19; ++i) {
                std::string skip;
                iss >> skip;
            }

            unsigned long vsize;
            long rss;
            iss >> vsize >> rss;
            info.rss = static_cast<uint64_t>(rss) * 4096;

            if (comm.front() == '(') comm = comm.substr(1);
            if (comm.back() == ')') comm.pop_back();
            info.comm = comm;
        }

        std::string taskPath = std::string("/proc/") + entry->d_name + "/task";
        DIR* taskDir = opendir(taskPath.c_str());
        if (taskDir) {
            struct dirent* taskEntry;
            while ((taskEntry = readdir(taskDir)) != nullptr) {
                if (taskEntry->d_type == DT_DIR) {
                    char* tEnd;
                    strtoul(taskEntry->d_name, &tEnd, 10);
                    if (*tEnd == '\0') info.threads++;
                }
            }
            closedir(taskDir);
        }

        std::string statusPath = std::string("/proc/") + entry->d_name + "/status";
        std::ifstream statusFile(statusPath);
        if (statusFile.is_open()) {
            std::string line;
            while (std::getline(statusFile, line)) {
                if (line.find("VmRSS:") == 0) {
                    std::istringstream iss(line);
                    std::string label;
                    unsigned long value;
                    std::string unit;
                    iss >> label >> value >> unit;
                    info.memPercent = static_cast<double>(value) / (1024.0 * 1024.0);
                }
            }
        }

        processes.push_back(info);
    }
    closedir(dir);

    return processes;
}

void printHeader() {
    std::cout << "\033[2J\033[H";
    std::cout << "Linux X-Ray Vision - Process Monitor (xray-top)" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    std::cout << std::left << std::setw(8) << "PID"
              << std::setw(8) << "PPID"
              << std::setw(16) << "NAME"
              << std::setw(8) << "CPU%"
              << std::setw(10) << "MEM(MB)"
              << std::setw(8) << "THR"
              << std::setw(4) << "S"
              << std::endl;
    std::cout << std::string(80, '-') << std::endl;
}

void printProcesses(const std::vector<ProcessInfo>& processes) {
    auto sorted = processes;
    std::sort(sorted.begin(), sorted.end(), 
              [](const auto& a, const auto& b) { return a.cpuPercent > b.cpuPercent; });

    for (size_t i = 0; i < std::min(sorted.size(), size_t(25)); ++i) {
        const auto& p = sorted[i];
        std::cout << std::left << std::setw(8) << p.pid
                  << std::setw(8) << p.ppid
                  << std::setw(16) << (p.comm.length() > 15 ? p.comm.substr(0, 15) : p.comm)
                  << std::setw(8) << std::fixed << std::setprecision(1) << p.cpuPercent
                  << std::setw(10) << std::fixed << std::setprecision(1) << p.memPercent
                  << std::setw(8) << p.threads
                  << std::setw(4) << p.state
                  << std::endl;
    }

    std::cout << std::string(80, '-') << std::endl;
    std::cout << "Total processes: " << processes.size() << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "Starting xray-top... Press Ctrl+C to exit." << std::endl;

    int delay = 2;
    if (argc > 1) {
        delay = std::atoi(argv[1]);
        if (delay < 1) delay = 1;
    }

    while (true) {
        auto processes = readProcesses();
        printHeader();
        printProcesses(processes);
        std::cout << "\nRefresh: " << delay << "s | Press Ctrl+C to quit" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(delay));
    }

    return 0;
}
