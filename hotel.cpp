#include <iostream>
#include <string>
#include <iomanip>
#include <limits>
using namespace std;

const int MAX = 100;

struct Booking {
    int id;
    string name, phone;
    int roomType, roomNo, nights;
    double foodBill;
    bool active;
};

class Hotel {
private:
    Booking bookings[MAX];
    int count;
    int nextId;
    bool occupied[4][5];

    string roomNames[4] = {
        "Single Non-AC", "Single AC",
        "Double AC", "Deluxe AC"
    };

    double roomPrices[4] = {1000, 1500, 2500, 4000};

    string foodNames[10] = {
        "Tea", "Coffee", "Sandwich", "Burger",
        "Pizza", "Pasta", "Rice", "Dal",
        "Paneer", "Cold Drink"
    };

    double foodPrices[10] = {
        30, 50, 80, 100, 180,
        150, 100, 120, 180, 40
    };

    int readInt(string message, int min, int max) {
        int value;
        while (true) {
            cout << message;
            if (cin >> value && value >= min && value <= max) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                return value;
            }
            cout << "Invalid input! Try again.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    string readText(string message) {
        string value;
        do {
            cout << message;
            getline(cin, value);
            if (value.empty())
                cout << "Input cannot be empty!\n";
        } while (value.empty());
        return value;
    }

    int findBooking(int id) {
        for (int i = 0; i < count; i++) {
            if (bookings[i].id == id && bookings[i].active)
                return i;
        }
        return -1;
    }

    int roomIndex(int roomNo) {
        if (roomNo >= 101 && roomNo <= 105) return 0;
        if (roomNo >= 201 && roomNo <= 205) return 1;
        if (roomNo >= 301 && roomNo <= 305) return 2;
        if (roomNo >= 401 && roomNo <= 405) return 3;
        return -1;
    }

    void showRoomTypes() {
        cout << "\n------ ROOM TYPES ------\n";
        for (int i = 0; i < 4; i++) {
            cout << i + 1 << ". " << roomNames[i]
                 << " - Rs. " << roomPrices[i] << "/night\n";
        }
    }

    void showAvailableRooms(int type) {
        cout << "\nAvailable rooms: ";
        bool found = false;

        for (int i = 0; i < 5; i++) {
            if (!occupied[type][i]) {
                cout << (type + 1) * 100 + i + 1 << " ";
                found = true;
            }
        }

        if (!found) cout << "None";
        cout << "\n";
    }

    void bookRoom() {
        if (count >= MAX) {
            cout << "Booking storage is full!\n";
            return;
        }

        showRoomTypes();
        int type = readInt("Select room type (1-4): ", 1, 4) - 1;

        showAvailableRooms(type);

        int availableRoom = -1;
        for (int i = 0; i < 5; i++) {
            if (!occupied[type][i]) {
                availableRoom = i;
                break;
            }
        }

        if (availableRoom == -1) {
            cout << "Sorry, no rooms available in this category.\n";
            return;
        }

        Booking b;
        b.id = nextId++;
        b.name = readText("Enter guest name: ");
        b.phone = readText("Enter phone number: ");
        b.nights = readInt("Number of nights (1-365): ", 1, 365);
        b.roomType = type;
        b.roomNo = (type + 1) * 100 + availableRoom + 1;
        b.foodBill = 0;
        b.active = true;

        occupied[type][availableRoom] = true;
        bookings[count++] = b;

        cout << "\nBooking successful!\n";
        cout << "Booking ID: " << b.id << "\n";
        cout << "Guest: " << b.name << "\n";
        cout << "Room: " << b.roomNo << "\n";
        cout << "Room Type: " << roomNames[type] << "\n";
        cout << "Room Rent: Rs. " << roomPrices[type] * b.nights << "\n";
    }

    void searchBooking() {
        int id = readInt("Enter booking ID: ", 1, 999999);
        int i = findBooking(id);

        if (i == -1) {
            cout << "Active booking not found.\n";
            return;
        }

        cout << "\n------ BOOKING DETAILS ------\n";
        cout << "Booking ID: " << bookings[i].id << "\n";
        cout << "Guest Name: " << bookings[i].name << "\n";
        cout << "Phone: " << bookings[i].phone << "\n";
        cout << "Room Number: " << bookings[i].roomNo << "\n";
        cout << "Room Type: " << roomNames[bookings[i].roomType] << "\n";
        cout << "Nights: " << bookings[i].nights << "\n";
        cout << "Food Bill: Rs. " << bookings[i].foodBill << "\n";
    }

    void orderFood() {
        int id = readInt("Enter booking ID: ", 1, 999999);
        int i = findBooking(id);

        if (i == -1) {
            cout << "Active booking not found.\n";
            return;
        }

        cout << "\n------ FOOD MENU ------\n";
        for (int j = 0; j < 10; j++) {
            cout << j + 1 << ". " << foodNames[j]
                 << " - Rs. " << foodPrices[j] << "\n";
        }
        cout << "0. Finish order\n";

        double total = 0;

        while (true) {
            int choice = readInt("Select food item (0-10): ", 0, 10);
            if (choice == 0) break;

            int qty = readInt("Enter quantity (1-50): ", 1, 50);
            double amount = foodPrices[choice - 1] * qty;
            total += amount;

            cout << foodNames[choice - 1]
                 << " x " << qty << " = Rs. " << amount << "\n";
        }

        bookings[i].foodBill += total;
        cout << "Food order added. Total: Rs. " << total << "\n";
        cout << "Updated food bill: Rs. "
             << bookings[i].foodBill << "\n";
    }

    void cancelBooking() {
        int id = readInt("Enter booking ID to cancel: ", 1, 999999);
        int i = findBooking(id);

        if (i == -1) {
            cout << "Active booking not found.\n";
            return;
        }

        int type = bookings[i].roomType;
        int room = bookings[i].roomNo % 100 - 1;

        occupied[type][room] = false;
        bookings[i].active = false;

        cout << "Booking cancelled successfully.\n";
    }

    void checkOut() {
        int id = readInt("Enter booking ID for checkout: ", 1, 999999);
        int i = findBooking(id);

        if (i == -1) {
            cout << "Active booking not found.\n";
            return;
        }

        double roomBill = roomPrices[bookings[i].roomType]
                          * bookings[i].nights;
        double subtotal = roomBill + bookings[i].foodBill;
        double tax = subtotal * 0.05;
        double total = subtotal + tax;

        cout << fixed << setprecision(2);
        cout << "\n========== HOTEL INVOICE ==========\n";
        cout << "Booking ID: " << bookings[i].id << "\n";
        cout << "Guest Name: " << bookings[i].name << "\n";
        cout << "Phone: " << bookings[i].phone << "\n";
        cout << "Room Number: " << bookings[i].roomNo << "\n";
        cout << "Room Type: " << roomNames[bookings[i].roomType] << "\n";
        cout << "Nights: " << bookings[i].nights << "\n";
        cout << "Room Charges: Rs. " << roomBill << "\n";
        cout << "Food Charges: Rs. " << bookings[i].foodBill << "\n";
        cout << "Subtotal: Rs. " << subtotal << "\n";
        cout << "Tax (5% assumed): Rs. " << tax << "\n";
        cout << "TOTAL AMOUNT: Rs. " << total << "\n";

        cout << "\nPayment Method:\n";
        cout << "1. Cash\n2. UPI\n3. Card\n";
        int payment = readInt("Select payment method (1-3): ", 1, 3);

        string methods[3] = {"Cash", "UPI", "Card"};
        cout << "Payment method recorded: " << methods[payment - 1] << "\n";
        cout << "Thank you for staying with us!\n";
        cout << "===================================\n";

        int type = bookings[i].roomType;
        int room = bookings[i].roomNo % 100 - 1;
        occupied[type][room] = false;
        bookings[i].active = false;
    }

    void adminReport() {
        int activeCount = 0;
        double revenue = 0;

        cout << "\n------ ADMIN REPORT ------\n";

        for (int i = 0; i < count; i++) {
            if (bookings[i].active) {
                activeCount++;
                revenue += roomPrices[bookings[i].roomType]
                           * bookings[i].nights;
                revenue += bookings[i].foodBill;

                cout << "ID: " << bookings[i].id
                     << " | Guest: " << bookings[i].name
                     << " | Room: " << bookings[i].roomNo
                     << " | Nights: " << bookings[i].nights << "\n";
            }
        }

        cout << "Active bookings: " << activeCount << "\n";
        cout << "Expected bill value of active bookings: Rs. "
             << revenue << "\n";
        cout << "Note: This is not actual collected revenue.\n";
    }

public:
    Hotel() {
        count = 0;
        nextId = 1001;

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 5; j++) {
                occupied[i][j] = false;
            }
        }
    }

    void run() {
        while (true) {
            cout << "\n\n================================\n";
            cout << "     HOTEL MANAGEMENT SYSTEM\n";
            cout << "================================\n";
            cout << "1. Book a Room\n";
            cout << "2. Search Booking\n";
            cout << "3. Order Food\n";
            cout << "4. Check Out and Generate Bill\n";
            cout << "5. Cancel Booking\n";
            cout << "6. Admin Report\n";
            cout << "0. Exit\n";

            int choice = readInt("Enter your choice: ", 0, 6);

            switch (choice) {
                case 1: bookRoom(); break;
                case 2: searchBooking(); break;
                case 3: orderFood(); break;
                case 4: checkOut(); break;
                case 5: cancelBooking(); break;
                case 6: adminReport(); break;
                case 0:
                    cout << "Thank you for using Hotel Management System!\n";
                    return;
            }
        }
    }
};

int main() {
    Hotel hotel;
    hotel.run();
    return 0;
}