#include <iostream>
#include <sqlite3.h>
#include <nlohmann/json.hpp>
#include "httplib.h"

using json = nlohmann::json;

int main() {
    sqlite3* db = nullptr;

    if (sqlite3_open("hotel.db", &db) != SQLITE_OK) {
        std::cerr << "Database open nahi hua.\n";

        if (db) {
            std::cerr << sqlite3_errmsg(db) << "\n";
            sqlite3_close(db);
        }

        return 1;
    }

    httplib::Server server;

    if (!server.set_mount_point("/", "../frontend")) {
        std::cerr << "Frontend folder nahi mila.\n";
        std::cerr << "Server backend folder se run karo.\n";
        sqlite3_close(db);
        return 1;
    }

    server.Get("/api/health",
        [](const httplib::Request&, httplib::Response& response) {
            json result = {
                {"message", "C++ backend is running"},
                {"database", "SQLite connected"}
            };

            response.set_content(
                result.dump(),
                "application/json"
            );
        }
    );

        server.Get("/api/rooms",
        [](const httplib::Request&, httplib::Response& response) {
            json rooms = json::array({
                {
                    {"type", 0},
                    {"name", "Single Non-AC"},
                    {"roomStart", 101},
                    {"roomEnd", 105},
                    {"pricePerNight", 1000}
                },
                {
                    {"type", 1},
                    {"name", "Single AC"},
                    {"roomStart", 201},
                    {"roomEnd", 205},
                    {"pricePerNight", 1500}
                },
                {
                    {"type", 2},
                    {"name", "Double AC"},
                    {"roomStart", 301},
                    {"roomEnd", 305},
                    {"pricePerNight", 2500}
                },
                {
                    {"type", 3},
                    {"name", "Deluxe AC"},
                    {"roomStart", 401},
                    {"roomEnd", 405},
                    {"pricePerNight", 4000}
                }
            });

            response.set_content(rooms.dump(), "application/json");
        }
    );

    std::cout << "Website: http://127.0.0.1:8080\n";
    std::cout << "API test: http://127.0.0.1:8080/api/health\n";
    std::cout << "Stop: Ctrl+C\n";

    bool success = server.listen("127.0.0.1", 8080);

    sqlite3_close(db);

    if (!success) {
        std::cerr << "Server start nahi hua. Port 8080 busy ho sakta hai.\n";
        return 1;
    }

    return 0;
}