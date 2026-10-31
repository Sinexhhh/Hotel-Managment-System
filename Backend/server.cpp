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

        const char* schema = R"(
        CREATE TABLE IF NOT EXISTS bookings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            phone TEXT NOT NULL,
            type INTEGER NOT NULL CHECK(type BETWEEN 0 AND 3),
            room INTEGER NOT NULL,
            nights INTEGER NOT NULL CHECK(nights BETWEEN 1 AND 365),
            food_bill INTEGER NOT NULL DEFAULT 0,
            active INTEGER NOT NULL DEFAULT 1 CHECK(active IN (0, 1))
        );
    )";

    char* error = nullptr;

    if (sqlite3_exec(db, schema, nullptr, nullptr, &error) != SQLITE_OK) {
        std::cerr << "Table creation error: "
                  << (error ? error : "Unknown error")
                  << "\n";

        sqlite3_free(error);
        sqlite3_close(db);
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

        server.Get("/api/stats",
        [](const httplib::Request&, httplib::Response& response) {
            sqlite3* connection = nullptr;

            if (sqlite3_open("hotel.db", &connection) != SQLITE_OK) {
                if (connection) {
                    sqlite3_close(connection);
                }

                response.status = 500;
                response.set_content(
                    R"({"error":"Database open failed"})",
                    "application/json"
                );
                return;
            }

            const char* sql = R"(
                SELECT
                    COUNT(*) AS total_bookings,
                    COALESCE(SUM(
                        CASE WHEN active = 1 THEN 1 ELSE 0 END
                    ), 0) AS active_bookings,
                    COALESCE(SUM(
                        CASE WHEN active = 1 THEN food_bill ELSE 0 END
                    ), 0) AS active_food_bill
                FROM bookings;
            )";

            sqlite3_stmt* statement = nullptr;

            if (sqlite3_prepare_v2(
                    connection, sql, -1, &statement, nullptr
                ) != SQLITE_OK) {
                sqlite3_close(connection);

                response.status = 500;
                response.set_content(
                    R"({"error":"Could not read statistics"})",
                    "application/json"
                );
                return;
            }

            json stats = {
                {"totalRooms", 20},
                {"totalBookings", 0},
                {"activeBookings", 0},
                {"availableRooms", 20},
                {"activeFoodBill", 0}
            };

            if (sqlite3_step(statement) == SQLITE_ROW) {
                int activeBookings = sqlite3_column_int(statement, 1);

                stats["totalBookings"] =
                    sqlite3_column_int(statement, 0);

                stats["activeBookings"] = activeBookings;

                stats["availableRooms"] = 20 - activeBookings;

                stats["activeFoodBill"] =
                    sqlite3_column_int(statement, 2);
            }

            sqlite3_finalize(statement);
            sqlite3_close(connection);

            response.set_content(stats.dump(), "application/json");
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