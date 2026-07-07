#include <iostream>
#include <spdlog/spdlog.h>
#include <cxxopts.hpp>
#include "engine/event_bus.hpp"
#include "engine/event_dispatcher.hpp"
#include "graph/graph.hpp"
#include "database/database.hpp"
#include "security/security_analyzer.hpp"

using namespace xray;

void printBanner() {
    std::cout << R"(
    _ _ _       _                 
   | (_) |     | |                
   | |_| | __ _| |__   _____  __  
   | | | |/ _` | '_ \ / _ \ \/ /  
   | | | | (_| | | | | (_) >  <   
   |_|_|_|\__, |_| |_|\___/_/\_\  
            __/ |                   
           |___/                    
   Linux X-Ray Vision - System Monitor
   Version 0.1.0
)" << std::endl;
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("xray", "Linux X-Ray Vision - System Monitor");

    options.add_options()
        ("h,help", "Print help")
        ("v,version", "Print version")
        ("g,gui", "Launch GUI")
        ("m,monitor", "Start continuous monitoring")
        ("r,record", "Record session to file", cxxopts::value<std::string>())
        ("p,play", "Replay session from file", cxxopts::value<std::string>())
        ("e,export", "Export data", cxxopts::value<std::string>())
        ("f,format", "Export format (json,csv,sqlite)", cxxopts::value<std::string>()->default_value("json"))
        ("d,database", "Database path", cxxopts::value<std::string>()->default_value("xray.db"))
        ("l,log-level", "Log level (trace,debug,info,warn,error)", cxxopts::value<std::string>()->default_value("info"))
        ("c,config", "Configuration file", cxxopts::value<std::string>())
        ("s,snapshot", "Create system snapshot")
        ("a,analyze", "Run AI analysis")
        ("t,time", "Monitoring duration in seconds", cxxopts::value<int>())
    ;

    try {
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            printBanner();
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (result.count("version")) {
            std::cout << "Linux X-Ray Vision 0.1.0" << std::endl;
            return 0;
        }

        std::string logLevel = result["log-level"].as<std::string>();
        if (logLevel == "trace") spdlog::set_level(spdlog::level::trace);
        else if (logLevel == "debug") spdlog::set_level(spdlog::level::debug);
        else if (logLevel == "info") spdlog::set_level(spdlog::level::info);
        else if (logLevel == "warn") spdlog::set_level(spdlog::level::warn);
        else if (logLevel == "error") spdlog::set_level(spdlog::level::err);

        printBanner();
        spdlog::info("Starting Linux X-Ray Vision");

        auto eventBus = std::make_shared<engine::EventBus>();
        auto dispatcher = std::make_shared<engine::EventDispatcher>(eventBus);
        auto graph = std::make_shared<graph::Graph>();
        auto database = std::make_shared<database::Database>(result["database"].as<std::string>());
        auto security = std::make_shared<security::SecurityAnalyzer>();

        if (!database->initialize()) {
            spdlog::error("Failed to initialize database");
            return 1;
        }

        eventBus->start();

        if (result.count("gui")) {
            spdlog::info("Launching GUI...");
            std::cout << "GUI mode requested. Build with Qt6 support enabled." << std::endl;
        }

        if (result.count("monitor")) {
            spdlog::info("Starting continuous monitoring...");
            int duration = result.count("time") ? result["time"].as<int>() : 0;

            eventBus->subscribeAll([&database, &security](const engine::Event& event) {
                database->storeEvent(event);
                security->analyzeEvent(event);
            });

            auto start = std::chrono::steady_clock::now();
            while (true) {
                if (duration > 0) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                        std::chrono::steady_clock::now() - start).count();
                    if (elapsed >= duration) break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        if (result.count("snapshot")) {
            spdlog::info("Creating system snapshot...");
        }

        if (result.count("analyze")) {
            spdlog::info("Running AI analysis...");
        }

        if (result.count("export")) {
            std::string path = result["export"].as<std::string>();
            std::string format = result["format"].as<std::string>();
            spdlog::info("Exporting data to {} in {} format", path, format);
        }

        eventBus->stop();
        spdlog::info("Linux X-Ray Vision shutdown complete");

    } catch (const cxxopts::exceptions::exception& e) {
        std::cerr << "Error parsing options: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        spdlog::error("Fatal error: {}", e.what());
        return 1;
    }

    return 0;
}
