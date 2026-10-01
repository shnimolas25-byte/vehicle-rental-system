// ============================================================================
//  VEHICLE RENTAL MANAGEMENT SYSTEM
//  Second Semester MIS Project - Single File Version
//
//  Prepared By : Nimolas Shrestha
//  Program     : BIT
//  Semester    : Second Semester
//
//  Data is saved in three text files placed next to this program:
//    vehicles.txt , customers.txt , rentals.txt
//  File format (pipe separated):
//    V001|BA 12 PA 1234|Toyota|Corolla|Sedan|2022|3000|Available
//    C001|Hari Prasad|9812345678|hari@gmail.com|Kathmandu|01-12345
//    R001|C001|V001|2026-09-21|2026-09-24|-|3|9000|0|9000|Pending|Active
// ============================================================================

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <cmath>
using namespace std;

// ============================================================================
//  INPUT HELPER FUNCTIONS
//  Common functions for taking input safely and other small utilities
// ============================================================================

// Removes spaces from the start and end of a line
string trim(const string &text)
{
    int start = 0;
    int end = (int)text.length() - 1;

    while (start <= end && isspace((unsigned char)text[start]))
        start++;

    while (end >= start && isspace((unsigned char)text[end]))
        end--;

    return text.substr(start, end - start + 1);
}

// Converts a string to lower case (used in searching)
string toLower(string text)
{
    for (int i = 0; i < (int)text.length(); i++)
        text[i] = tolower((unsigned char)text[i]);
    return text;
}

// Reads a line and refuses empty input
string readLineNonEmpty(const string &prompt)
{
    string line;
    while (true)
    {
        cout << prompt;
        getline(cin, line);
        line = trim(line);
        if (!line.empty())
            return line;
        cout << "This field cannot be empty." << endl;
    }
}

// Reads an integer inside a range with error handling
int readInt(const string &prompt, int minVal, int maxVal)
{
    string line;
    while (true)
    {
        cout << prompt;
        getline(cin, line);
        line = trim(line);

        stringstream ss(line);
        int value;
        char extra;

        if (ss >> value && !(ss >> extra) && value >= minVal && value <= maxVal)
            return value;

        cout << "Invalid input. Please enter a number between "
             << minVal << " and " << maxVal << "." << endl;
    }
}

// Reads a double value with a minimum limit and custom error message
double readDouble(const string &prompt, double minVal, const string &errorMsg)
{
    string line;
    while (true)
    {
        cout << prompt;
        getline(cin, line);
        line = trim(line);

        stringstream ss(line);
        double value;
        char extra;

        if (ss >> value && !(ss >> extra))
        {
            if (value >= minVal)
                return value;
            cout << errorMsg << endl;
        }
        else
        {
            cout << "Invalid input. Please enter a valid number." << endl;
        }
    }
}

// Reads a menu choice between 1 and maxChoice
int readMenuChoice(int maxChoice)
{
    string line;
    while (true)
    {
        cout << "Enter your choice: ";
        getline(cin, line);
        line = trim(line);

        stringstream ss(line);
        int value;
        char extra;

        if (ss >> value && !(ss >> extra) && value >= 1 && value <= maxChoice)
            return value;

        cout << "Invalid choice. Please enter a valid option." << endl;
    }
}

// Asks the user a Yes/No question
bool askConfirmation(const string &prompt)
{
    string line;
    while (true)
    {
        cout << prompt;
        getline(cin, line);
        line = toLower(trim(line));

        if (line == "y" || line == "yes")
            return true;
        if (line == "n" || line == "no")
            return false;

        cout << "Please enter Y or N." << endl;
    }
}

// Waits until the user presses Enter
void pauseConsole()
{
    cout << endl << "Press Enter to continue...";
    string line;
    getline(cin, line);
}

// Splits a line into parts using a delimiter (used for reading files)
vector<string> splitString(const string &line, char delimiter)
{
    vector<string> parts;
    stringstream ss(line);
    string item;

    while (getline(ss, item, delimiter))
        parts.push_back(item);

    return parts;
}

// ============================================================================
//  DATE HELPER FUNCTIONS
//  Used for validating dates and calculating rental days and late fines
// ============================================================================

// Fine charged for each late day (college-project rule)
const double LATE_FINE_PER_DAY = 500.0;

// Name of the data files (kept in the same folder as this program)
const string VEHICLE_FILE = "vehicles.txt";
const string CUSTOMER_FILE = "customers.txt";
const string RENTAL_FILE = "rentals.txt";

// Returns the number of days in a month (handles leap years)
static int daysInMonth(int month, int year)
{
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0))
        return 29;
    return days[month - 1];
}

// Checks if a date string has the format YYYY-MM-DD and is a real date
bool isValidDate(const string &date)
{
    if (date.length() != 10)
        return false;
    if (date[4] != '-' || date[7] != '-')
        return false;

    for (int i = 0; i < 10; i++)
    {
        if (i != 4 && i != 7 && !isdigit((unsigned char)date[i]))
            return false;
    }

    int year;
    int month;
    int day;
    stringstream dateStream(date.substr(0, 4) + " " + date.substr(5, 2) + " " + date.substr(8, 2));
    dateStream >> year >> month >> day;

    if (year < 1900 || year > 2100)
        return false;
    if (month < 1 || month > 12)
        return false;
    if (day < 1 || day > daysInMonth(month, year))
        return false;

    return true;
}

// Converts a date to a total number of days since year 1900
static int dateToDays(const string &date)
{
    int year;
    int month;
    int day;
    stringstream dateStream(date.substr(0, 4) + " " + date.substr(5, 2) + " " + date.substr(8, 2));
    dateStream >> year >> month >> day;

    int total = 0;

    for (int y = 1900; y < year; y++)
    {
        if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)
            total += 366;
        else
            total += 365;
    }

    for (int m = 1; m < month; m++)
        total += daysInMonth(m, year);

    total += day;

    return total;
}

// Returns (endDate - startDate) in days
int daysBetween(const string &startDate, const string &endDate)
{
    return dateToDays(endDate) - dateToDays(startDate);
}

// ============================================================================
//  VEHICLE CLASS
//  Holds one vehicle record (car, bike, etc.)
// ============================================================================

class Vehicle
{
private:
    string vehicleId;
    string vehicleNumber;
    string brand;
    string model;
    string type;
    int year;
    double dailyRate;
    bool available;

public:
    Vehicle();
    Vehicle(string vehicleId, string vehicleNumber, string brand, string model,
            string type, int year, double dailyRate, bool available);

    // Getters
    string getVehicleId() const;
    string getVehicleNumber() const;
    string getBrand() const;
    string getModel() const;
    string getType() const;
    int getYear() const;
    double getDailyRate() const;
    bool isAvailable() const;

    // Setters
    void setVehicleId(const string &vehicleId);
    void setVehicleNumber(const string &vehicleNumber);
    void setBrand(const string &brand);
    void setModel(const string &model);
    void setType(const string &type);
    void setYear(int year);
    void setDailyRate(double dailyRate);
    void setAvailable(bool available);

    // Other functions
    void input();
    void display() const;
    static void displayHeader();
    string getStatusText() const;
    string toFileString() const;
    static Vehicle fromFileString(const string &line);
};

// Default constructor: a new vehicle is always available
Vehicle::Vehicle()
{
    vehicleId = "";
    vehicleNumber = "";
    brand = "";
    model = "";
    type = "";
    year = 0;
    dailyRate = 0.0;
    available = true;
}

// Parameterized constructor
Vehicle::Vehicle(string vehicleId, string vehicleNumber, string brand, string model,
                 string type, int year, double dailyRate, bool available)
{
    this->vehicleId = vehicleId;
    this->vehicleNumber = vehicleNumber;
    this->brand = brand;
    this->model = model;
    this->type = type;
    this->year = year;
    this->dailyRate = dailyRate;
    this->available = available;
}

// Getters
string Vehicle::getVehicleId() const       { return vehicleId; }
string Vehicle::getVehicleNumber() const   { return vehicleNumber; }
string Vehicle::getBrand() const           { return brand; }
string Vehicle::getModel() const           { return model; }
string Vehicle::getType() const            { return type; }
int Vehicle::getYear() const               { return year; }
double Vehicle::getDailyRate() const       { return dailyRate; }
bool Vehicle::isAvailable() const          { return available; }

// Setters
void Vehicle::setVehicleId(const string &vehicleId)       { this->vehicleId = vehicleId; }
void Vehicle::setVehicleNumber(const string &vehicleNumber) { this->vehicleNumber = vehicleNumber; }
void Vehicle::setBrand(const string &brand)               { this->brand = brand; }
void Vehicle::setModel(const string &model)               { this->model = model; }
void Vehicle::setType(const string &type)                 { this->type = type; }
void Vehicle::setYear(int year)                           { this->year = year; }
void Vehicle::setDailyRate(double dailyRate)              { this->dailyRate = dailyRate; }
void Vehicle::setAvailable(bool available)                { this->available = available; }

// Reads all vehicle details from the user (format validation only,
// duplicate checks are done by the main system)
void Vehicle::input()
{
    vehicleId = readLineNonEmpty("Vehicle ID: ");
    vehicleNumber = readLineNonEmpty("Vehicle Number: ");
    brand = readLineNonEmpty("Brand: ");
    model = readLineNonEmpty("Model: ");
    type = readLineNonEmpty("Type: ");
    year = readInt("Year: ", 1900, 2100);
    dailyRate = readDouble("Daily Rental Rate: ", 0, "Rental rate cannot be negative.");
    available = true;
}

// Converts the boolean flag to text used for display and storage
string Vehicle::getStatusText() const
{
    return available ? "Available" : "Rented";
}

// Prints the table header for a vehicle list
void Vehicle::displayHeader()
{
    cout << left
         << setw(8) << "ID"
         << setw(17) << "Number"
         << setw(13) << "Brand"
         << setw(13) << "Model"
         << setw(10) << "Type"
         << setw(7) << "Year"
         << setw(9) << "Rate"
         << "Status" << endl;
    cout << string(80, '-') << endl;
}

// Prints one vehicle as a table row
void Vehicle::display() const
{
    cout << left
         << setw(8) << vehicleId
         << setw(17) << vehicleNumber
         << setw(13) << brand
         << setw(13) << model
         << setw(10) << type
         << setw(7) << year
         << setw(9) << dailyRate
         << getStatusText() << endl;
}

// Converts the vehicle into one file line
string Vehicle::toFileString() const
{
    stringstream ss;
    ss << vehicleId << "|"
       << vehicleNumber << "|"
       << brand << "|"
       << model << "|"
       << type << "|"
       << year << "|"
       << dailyRate << "|"
       << getStatusText();
    return ss.str();
}

// Reads a vehicle from one file line
Vehicle Vehicle::fromFileString(const string &line)
{
    vector<string> parts = splitString(line, '|');

    Vehicle v;
    v.vehicleId = parts[0];
    v.vehicleNumber = parts[1];
    v.brand = parts[2];
    v.model = parts[3];
    v.type = parts[4];
    stringstream ss(parts[5] + " " + parts[6]);
    ss >> v.year >> v.dailyRate;
    v.available = (parts[7] == "Available");

    return v;
}

// ============================================================================
//  CUSTOMER CLASS
//  Holds one customer record
// ============================================================================

class Customer
{
private:
    string customerId;
    string name;
    string phone;
    string email;
    string address;
    string licenseNumber;

public:
    Customer();
    Customer(string customerId, string name, string phone, string email,
             string address, string licenseNumber);

    // Getters
    string getCustomerId() const;
    string getName() const;
    string getPhone() const;
    string getEmail() const;
    string getAddress() const;
    string getLicenseNumber() const;

    // Setters
    void setCustomerId(const string &customerId);
    void setName(const string &name);
    void setPhone(const string &phone);
    void setEmail(const string &email);
    void setAddress(const string &address);
    void setLicenseNumber(const string &licenseNumber);

    // Other functions
    void input();
    void display() const;
    static void displayHeader();
    string toFileString() const;
    static Customer fromFileString(const string &line);
};

// Default constructor
Customer::Customer()
{
    customerId = "";
    name = "";
    phone = "";
    email = "";
    address = "";
    licenseNumber = "";
}

// Parameterized constructor
Customer::Customer(string customerId, string name, string phone, string email,
                   string address, string licenseNumber)
{
    this->customerId = customerId;
    this->name = name;
    this->phone = phone;
    this->email = email;
    this->address = address;
    this->licenseNumber = licenseNumber;
}

// Getters
string Customer::getCustomerId() const      { return customerId; }
string Customer::getName() const            { return name; }
string Customer::getPhone() const           { return phone; }
string Customer::getEmail() const           { return email; }
string Customer::getAddress() const         { return address; }
string Customer::getLicenseNumber() const   { return licenseNumber; }

// Setters
void Customer::setCustomerId(const string &customerId)       { this->customerId = customerId; }
void Customer::setName(const string &name)                   { this->name = name; }
void Customer::setPhone(const string &phone)                 { this->phone = phone; }
void Customer::setEmail(const string &email)                 { this->email = email; }
void Customer::setAddress(const string &address)             { this->address = address; }
void Customer::setLicenseNumber(const string &licenseNumber) { this->licenseNumber = licenseNumber; }

// Reads all customer details from the user (duplicate checks done by the system)
void Customer::input()
{
    customerId = readLineNonEmpty("Customer ID: ");
    name = readLineNonEmpty("Name: ");
    phone = readLineNonEmpty("Phone: ");
    email = readLineNonEmpty("Email: ");
    address = readLineNonEmpty("Address: ");
    licenseNumber = readLineNonEmpty("Driving License Number: ");
}

// Prints the table header for a customer list
void Customer::displayHeader()
{
    cout << left
         << setw(8) << "ID"
         << setw(20) << "Name"
         << setw(14) << "Phone"
         << setw(22) << "Email"
         << setw(17) << "Address"
         << "License" << endl;
    cout << string(95, '-') << endl;
}

// Prints one customer as a table row
void Customer::display() const
{
    cout << left
         << setw(8) << customerId
         << setw(20) << name
         << setw(14) << phone
         << setw(22) << email
         << setw(17) << address
         << licenseNumber << endl;
}

// Converts the customer into one file line
string Customer::toFileString() const
{
    stringstream ss;
    ss << customerId << "|"
       << name << "|"
       << phone << "|"
       << email << "|"
       << address << "|"
       << licenseNumber;
    return ss.str();
}

// Reads a customer from one file line
Customer Customer::fromFileString(const string &line)
{
    vector<string> parts = splitString(line, '|');

    Customer c;
    c.customerId = parts[0];
    c.name = parts[1];
    c.phone = parts[2];
    c.email = parts[3];
    c.address = parts[4];
    c.licenseNumber = parts[5];

    return c;
}

// ============================================================================
//  RENTAL CLASS
//  Holds one rental record (rent and return of a vehicle)
// ============================================================================

class Rental
{
private:
    string rentalId;
    string customerId;
    string vehicleId;
    string rentalDate;
    string expectedReturnDate;
    string actualReturnDate;
    int rentalDays;
    double rentalCharge;
    double fine;
    double totalAmount;
    string paymentStatus;
    string rentalStatus;

public:
    Rental();
    Rental(string rentalId, string customerId, string vehicleId,
           string rentalDate, string expectedReturnDate, string actualReturnDate,
           int rentalDays, double rentalCharge, double fine, double totalAmount,
           string paymentStatus, string rentalStatus);

    // Getters
    string getRentalId() const;
    string getCustomerId() const;
    string getVehicleId() const;
    string getRentalDate() const;
    string getExpectedReturnDate() const;
    string getActualReturnDate() const;
    int getRentalDays() const;
    double getRentalCharge() const;
    double getFine() const;
    double getTotalAmount() const;
    string getPaymentStatus() const;
    string getRentalStatus() const;

    // Setters
    void setCustomerId(const string &customerId);
    void setVehicleId(const string &vehicleId);
    void setRentalDate(const string &rentalDate);
    void setExpectedReturnDate(const string &expectedReturnDate);
    void setActualReturnDate(const string &actualReturnDate);
    void setRentalDays(int rentalDays);
    void setRentalCharge(double rentalCharge);
    void setFine(double fine);
    void setTotalAmount(double totalAmount);
    void setPaymentStatus(const string &paymentStatus);
    void setRentalStatus(const string &rentalStatus);

    // Status checks
    bool isActive() const;
    bool isCompleted() const;
    bool isCancelled() const;

    // Calculation functions
    void calculateRentalCharge(double dailyRate);
    void calculateFine(const string &expectedDate, const string &actualDate);
    void calculateTotal();

    // Display and storage
    void display() const;
    static void displayHeader();
    string toFileString() const;
    static Rental fromFileString(const string &line);
};

// Default constructor
Rental::Rental()
{
    rentalId = "";
    customerId = "";
    vehicleId = "";
    rentalDate = "";
    expectedReturnDate = "";
    actualReturnDate = "";
    rentalDays = 0;
    rentalCharge = 0.0;
    fine = 0.0;
    totalAmount = 0.0;
    paymentStatus = "Pending";
    rentalStatus = "Active";
}

// Parameterized constructor
Rental::Rental(string rentalId, string customerId, string vehicleId,
               string rentalDate, string expectedReturnDate, string actualReturnDate,
               int rentalDays, double rentalCharge, double fine, double totalAmount,
               string paymentStatus, string rentalStatus)
{
    this->rentalId = rentalId;
    this->customerId = customerId;
    this->vehicleId = vehicleId;
    this->rentalDate = rentalDate;
    this->expectedReturnDate = expectedReturnDate;
    this->actualReturnDate = actualReturnDate;
    this->rentalDays = rentalDays;
    this->rentalCharge = rentalCharge;
    this->fine = fine;
    this->totalAmount = totalAmount;
    this->paymentStatus = paymentStatus;
    this->rentalStatus = rentalStatus;
}

// Getters
string Rental::getRentalId() const             { return rentalId; }
string Rental::getCustomerId() const           { return customerId; }
string Rental::getVehicleId() const            { return vehicleId; }
string Rental::getRentalDate() const           { return rentalDate; }
string Rental::getExpectedReturnDate() const   { return expectedReturnDate; }
string Rental::getActualReturnDate() const     { return actualReturnDate; }
int Rental::getRentalDays() const              { return rentalDays; }
double Rental::getRentalCharge() const         { return rentalCharge; }
double Rental::getFine() const                 { return fine; }
double Rental::getTotalAmount() const          { return totalAmount; }
string Rental::getPaymentStatus() const        { return paymentStatus; }
string Rental::getRentalStatus() const         { return rentalStatus; }

// Setters
void Rental::setCustomerId(const string &customerId)          { this->customerId = customerId; }
void Rental::setVehicleId(const string &vehicleId)            { this->vehicleId = vehicleId; }
void Rental::setRentalDate(const string &rentalDate)          { this->rentalDate = rentalDate; }
void Rental::setExpectedReturnDate(const string &expectedDate) { this->expectedReturnDate = expectedDate; }
void Rental::setActualReturnDate(const string &actualDate)    { this->actualReturnDate = actualDate; }
void Rental::setRentalDays(int rentalDays)                    { this->rentalDays = rentalDays; }
void Rental::setRentalCharge(double rentalCharge)             { this->rentalCharge = rentalCharge; }
void Rental::setFine(double fine)                             { this->fine = fine; }
void Rental::setTotalAmount(double totalAmount)               { this->totalAmount = totalAmount; }
void Rental::setPaymentStatus(const string &paymentStatus)    { this->paymentStatus = paymentStatus; }
void Rental::setRentalStatus(const string &rentalStatus)      { this->rentalStatus = rentalStatus; }

// Status checks
bool Rental::isActive() const     { return rentalStatus == "Active"; }
bool Rental::isCompleted() const  { return rentalStatus == "Completed"; }
bool Rental::isCancelled() const  { return rentalStatus == "Cancelled"; }

// Rental charge = daily rate x number of days
void Rental::calculateRentalCharge(double dailyRate)
{
    rentalCharge = rentalDays * dailyRate;
}

// Fine is charged only if the vehicle is returned after the expected date
void Rental::calculateFine(const string &expectedDate, const string &actualDate)
{
    int lateDays = daysBetween(expectedDate, actualDate);
    if (lateDays > 0)
        fine = lateDays * LATE_FINE_PER_DAY;
    else
        fine = 0.0;
}

// Total amount = rental charge + late fine
void Rental::calculateTotal()
{
    totalAmount = rentalCharge + fine;
}

// Prints the table header for a rental list
void Rental::displayHeader()
{
    cout << left
         << setw(7) << "ID"
         << setw(9) << "Customer"
         << setw(9) << "Vehicle"
         << setw(12) << "Rental Date"
         << setw(12) << "Expected"
         << setw(12) << "Actual"
         << setw(6) << "Days"
         << setw(10) << "Charge"
         << setw(9) << "Fine"
         << setw(11) << "Total"
         << setw(10) << "Payment"
         << "Status" << endl;
    cout << string(115, '-') << endl;
}

// Prints one rental as a table row
void Rental::display() const
{
    cout << left
         << setw(7) << rentalId
         << setw(9) << customerId
         << setw(9) << vehicleId
         << setw(12) << rentalDate
         << setw(12) << expectedReturnDate
         << setw(12) << actualReturnDate
         << setw(6) << rentalDays
         << setw(10) << rentalCharge
         << setw(9) << fine
         << setw(11) << totalAmount
         << setw(10) << paymentStatus
         << rentalStatus << endl;
}

// Converts the rental into one file line
string Rental::toFileString() const
{
    stringstream ss;
    ss << rentalId << "|"
       << customerId << "|"
       << vehicleId << "|"
       << rentalDate << "|"
       << expectedReturnDate << "|"
       << actualReturnDate << "|"
       << rentalDays << "|"
       << rentalCharge << "|"
       << fine << "|"
       << totalAmount << "|"
       << paymentStatus << "|"
       << rentalStatus;
    return ss.str();
}

// Reads a rental from one file line
Rental Rental::fromFileString(const string &line)
{
    vector<string> parts = splitString(line, '|');

    Rental r;
    r.rentalId = parts[0];
    r.customerId = parts[1];
    r.vehicleId = parts[2];
    r.rentalDate = parts[3];
    r.expectedReturnDate = parts[4];
    r.actualReturnDate = parts[5];
    stringstream ss(parts[6] + " " + parts[7] + " " + parts[8] + " " + parts[9]);
    ss >> r.rentalDays >> r.rentalCharge >> r.fine >> r.totalAmount;
    r.paymentStatus = parts[10];
    r.rentalStatus = parts[11];

    return r;
}

// ============================================================================
//  FILE MANAGER CLASS
//  Handles all permanent storage using the text files
// ============================================================================

class FileManager
{
public:
    void loadVehicles(vector<Vehicle> &vehicles);
    void saveVehicles(const vector<Vehicle> &vehicles);
    void loadCustomers(vector<Customer> &customers);
    void saveCustomers(const vector<Customer> &customers);
    void loadRentals(vector<Rental> &rentals);
    void saveRentals(const vector<Rental> &rentals);
};

// Removes a trailing carriage return if present (handles Windows files)
static string cleanLine(string line)
{
    if (!line.empty() && line[line.length() - 1] == '\r')
        line.erase(line.length() - 1);
    return line;
}

// Loads vehicles from file. Creates the file if it does not exist.
void FileManager::loadVehicles(vector<Vehicle> &vehicles)
{
    ifstream file(VEHICLE_FILE.c_str());
    if (!file.is_open())
    {
        ofstream create(VEHICLE_FILE.c_str());
        create.close();
        return;
    }

    string line;
    while (getline(file, line))
    {
        line = cleanLine(line);
        if (line.empty())
            continue;
        vehicles.push_back(Vehicle::fromFileString(line));
    }
    file.close();
}

// Writes all vehicles back into the file (whole file is rewritten)
void FileManager::saveVehicles(const vector<Vehicle> &vehicles)
{
    ofstream file(VEHICLE_FILE.c_str());
    for (int i = 0; i < (int)vehicles.size(); i++)
        file << vehicles[i].toFileString() << endl;
    file.close();
}

// Loads customers from file. Creates the file if it does not exist.
void FileManager::loadCustomers(vector<Customer> &customers)
{
    ifstream file(CUSTOMER_FILE.c_str());
    if (!file.is_open())
    {
        ofstream create(CUSTOMER_FILE.c_str());
        create.close();
        return;
    }

    string line;
    while (getline(file, line))
    {
        line = cleanLine(line);
        if (line.empty())
            continue;
        customers.push_back(Customer::fromFileString(line));
    }
    file.close();
}

// Writes all customers back into the file (whole file is rewritten)
void FileManager::saveCustomers(const vector<Customer> &customers)
{
    ofstream file(CUSTOMER_FILE.c_str());
    for (int i = 0; i < (int)customers.size(); i++)
        file << customers[i].toFileString() << endl;
    file.close();
}

// Loads rentals from file. Creates the file if it does not exist.
void FileManager::loadRentals(vector<Rental> &rentals)
{
    ifstream file(RENTAL_FILE.c_str());
    if (!file.is_open())
    {
        ofstream create(RENTAL_FILE.c_str());
        create.close();
        return;
    }

    string line;
    while (getline(file, line))
    {
        line = cleanLine(line);
        if (line.empty())
            continue;
        rentals.push_back(Rental::fromFileString(line));
    }
    file.close();
}

// Writes all rentals back into the file (whole file is rewritten)
void FileManager::saveRentals(const vector<Rental> &rentals)
{
    ofstream file(RENTAL_FILE.c_str());
    for (int i = 0; i < (int)rentals.size(); i++)
        file << rentals[i].toFileString() << endl;
    file.close();
}

// ============================================================================
//  REPORT MANAGER CLASS
//  Prints summary reports to the console
// ============================================================================

class ReportManager
{
public:
    void vehicleSummary(const vector<Vehicle> &vehicles);
    void customerSummary(const vector<Customer> &customers, const vector<Rental> &rentals);
    void rentalSummary(const vector<Rental> &rentals);
    void incomeSummary(const vector<Rental> &rentals);
    void fineSummary(const vector<Rental> &rentals);
    void outstandingPayments(const vector<Rental> &rentals, const vector<Customer> &customers);
    void completeSummary(const vector<Vehicle> &vehicles, const vector<Customer> &customers,
                         const vector<Rental> &rentals);
};

// Prints a simple section heading
static void reportHeading(const string &title)
{
    cout << endl;
    cout << "==================================================" << endl;
    cout << title << endl;
    cout << "==================================================" << endl;
}

// Vehicle summary: total, available, rented and type-wise count
void ReportManager::vehicleSummary(const vector<Vehicle> &vehicles)
{
    reportHeading("VEHICLE SUMMARY");

    if (vehicles.empty())
    {
        cout << "No vehicles found." << endl;
        return;
    }

    int available = 0;
    int rented = 0;
    double totalValue = 0;

    for (int i = 0; i < (int)vehicles.size(); i++)
    {
        if (vehicles[i].isAvailable())
            available++;
        else
            rented++;
        totalValue += vehicles[i].getDailyRate();
    }

    cout << "Total Vehicles     : " << vehicles.size() << endl;
    cout << "Available Vehicles : " << available << endl;
    cout << "Rented Vehicles    : " << rented << endl;
    cout << "Average Daily Rate : Rs. " << totalValue / vehicles.size() << endl;

    cout << endl << "--- Vehicles by Type ---" << endl;
    cout << left << setw(12) << "Type" << "Count" << endl;
    cout << string(30, '-') << endl;

    for (int i = 0; i < (int)vehicles.size(); i++)
    {
        bool alreadyShown = false;
        for (int j = 0; j < i; j++)
        {
            if (vehicles[j].getType() == vehicles[i].getType())
                alreadyShown = true;
        }
        if (alreadyShown)
            continue;

        int count = 0;
        for (int k = 0; k < (int)vehicles.size(); k++)
        {
            if (vehicles[k].getType() == vehicles[i].getType())
                count++;
        }
        cout << left << setw(12) << vehicles[i].getType() << count << endl;
    }
}

// Customer summary: total customers and how many have active rentals
void ReportManager::customerSummary(const vector<Customer> &customers, const vector<Rental> &rentals)
{
    reportHeading("CUSTOMER SUMMARY");

    if (customers.empty())
    {
        cout << "No customers found." << endl;
        return;
    }

    int activeCustomers = 0;

    for (int i = 0; i < (int)customers.size(); i++)
    {
        for (int j = 0; j < (int)rentals.size(); j++)
        {
            if (rentals[j].getCustomerId() == customers[i].getCustomerId() &&
                rentals[j].isActive())
            {
                activeCustomers++;
                break;
            }
        }
    }

    cout << "Total Customers             : " << customers.size() << endl;
    cout << "Customers with Active Rental: " << activeCustomers << endl;
    cout << "Customers with No Active    : " << customers.size() - activeCustomers << endl;
}

// Rental summary: total, active, completed and cancelled rentals
void ReportManager::rentalSummary(const vector<Rental> &rentals)
{
    reportHeading("RENTAL SUMMARY");

    if (rentals.empty())
    {
        cout << "No rental records found." << endl;
        return;
    }

    int active = 0;
    int completed = 0;
    int cancelled = 0;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].isActive())
            active++;
        else if (rentals[i].isCompleted())
            completed++;
        else if (rentals[i].isCancelled())
            cancelled++;
    }

    cout << "Total Rentals     : " << rentals.size() << endl;
    cout << "Active Rentals    : " << active << endl;
    cout << "Completed Rentals : " << completed << endl;
    cout << "Cancelled Rentals : " << cancelled << endl;
}

// Income summary: total charges, fines, amount received and pending
void ReportManager::incomeSummary(const vector<Rental> &rentals)
{
    reportHeading("INCOME SUMMARY");

    if (rentals.empty())
    {
        cout << "No rental records found." << endl;
        return;
    }

    double totalCharges = 0;
    double totalFines = 0;
    double received = 0;
    double pending = 0;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (!rentals[i].isCompleted())
            continue;

        totalCharges += rentals[i].getRentalCharge();
        totalFines += rentals[i].getFine();

        if (rentals[i].getPaymentStatus() == "Paid")
            received += rentals[i].getTotalAmount();
        else
            pending += rentals[i].getTotalAmount();
    }

    cout << "Total Rental Charges (Completed): Rs. " << totalCharges << endl;
    cout << "Total Late Fines                : Rs. " << totalFines << endl;
    cout << "Amount Received (Paid)          : Rs. " << received << endl;
    cout << "Amount Pending                  : Rs. " << pending << endl;
}

// Fine summary: lists every completed rental that has a late fine
void ReportManager::fineSummary(const vector<Rental> &rentals)
{
    reportHeading("FINE SUMMARY");

    double totalFines = 0;
    int fineCount = 0;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].getFine() > 0)
        {
            totalFines += rentals[i].getFine();
            fineCount++;
        }
    }

    cout << "Rentals with Late Fine : " << fineCount << endl;
    cout << "Total Fine Amount      : Rs. " << totalFines << endl;

    if (fineCount == 0)
    {
        cout << "No late returns recorded." << endl;
        return;
    }

    cout << endl << "--- Late Returned Rentals ---" << endl;
    cout << left
         << setw(10) << "Rental ID"
         << setw(12) << "Vehicle"
         << setw(12) << "Expected"
         << setw(12) << "Actual"
         << setw(9) << "Late Days"
         << "Fine" << endl;
    cout << string(60, '-') << endl;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].getFine() <= 0)
            continue;

        int lateDays = daysBetween(rentals[i].getExpectedReturnDate(),
                                   rentals[i].getActualReturnDate());

        cout << left
             << setw(10) << rentals[i].getRentalId()
             << setw(12) << rentals[i].getVehicleId()
             << setw(12) << rentals[i].getExpectedReturnDate()
             << setw(12) << rentals[i].getActualReturnDate()
             << setw(9) << lateDays
             << rentals[i].getFine() << endl;
    }
}

// Outstanding payment: completed rentals whose payment is still pending
void ReportManager::outstandingPayments(const vector<Rental> &rentals, const vector<Customer> &customers)
{
    reportHeading("OUTSTANDING PAYMENTS");

    double totalPending = 0;
    int pendingCount = 0;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].isCompleted() && rentals[i].getPaymentStatus() == "Pending")
            pendingCount++;
    }

    if (pendingCount == 0)
    {
        cout << "No outstanding payments." << endl;
        return;
    }

    cout << "Pending Payment Records : " << pendingCount << endl << endl;
    cout << left
         << setw(10) << "Rental ID"
         << setw(12) << "Customer"
         << setw(12) << "Vehicle"
         << setw(12) << "Return Date"
         << "Amount" << endl;
    cout << string(60, '-') << endl;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (!rentals[i].isCompleted() || rentals[i].getPaymentStatus() != "Pending")
            continue;

        string customerName = "Unknown";
        for (int j = 0; j < (int)customers.size(); j++)
        {
            if (customers[j].getCustomerId() == rentals[i].getCustomerId())
            {
                customerName = customers[j].getName();
                break;
            }
        }

        cout << left
             << setw(10) << rentals[i].getRentalId()
             << setw(12) << customerName
             << setw(12) << rentals[i].getVehicleId()
             << setw(12) << rentals[i].getActualReturnDate()
             << rentals[i].getTotalAmount() << endl;

        totalPending += rentals[i].getTotalAmount();
    }

    cout << string(60, '-') << endl;
    cout << left << setw(46) << "Total Outstanding:" << "Rs. " << totalPending << endl;
}

// Complete system summary: one big report of everything
void ReportManager::completeSummary(const vector<Vehicle> &vehicles, const vector<Customer> &customers,
                                    const vector<Rental> &rentals)
{
    reportHeading("SYSTEM SUMMARY");

    int available = 0;
    int rented = 0;
    for (int i = 0; i < (int)vehicles.size(); i++)
    {
        if (vehicles[i].isAvailable())
            available++;
        else
            rented++;
    }

    int active = 0;
    int completed = 0;
    int cancelled = 0;
    double totalCharges = 0;
    double totalFines = 0;
    double totalReceived = 0;
    double totalPending = 0;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].isActive())
            active++;
        else if (rentals[i].isCompleted())
        {
            completed++;
            totalCharges += rentals[i].getRentalCharge();
            totalFines += rentals[i].getFine();
            if (rentals[i].getPaymentStatus() == "Paid")
                totalReceived += rentals[i].getTotalAmount();
            else
                totalPending += rentals[i].getTotalAmount();
        }
        else if (rentals[i].isCancelled())
            cancelled++;
    }

    cout << "Total Vehicles      : " << vehicles.size() << endl;
    cout << "Available Vehicles  : " << available << endl;
    cout << "Rented Vehicles     : " << rented << endl;
    cout << endl;
    cout << "Total Customers     : " << customers.size() << endl;
    cout << endl;
    cout << "Total Rentals       : " << rentals.size() << endl;
    cout << "Active Rentals      : " << active << endl;
    cout << "Completed Rentals   : " << completed << endl;
    cout << "Cancelled Rentals   : " << cancelled << endl;
    cout << endl;
    cout << "Total Rental Charges: Rs. " << totalCharges << endl;
    cout << "Total Fines         : Rs. " << totalFines << endl;
    cout << "Payment Received    : Rs. " << totalReceived << endl;
    cout << "Pending Payments    : Rs. " << totalPending << endl;
    cout << "==================================================" << endl;
}

// ============================================================================
//  MAIN SYSTEM CLASS
//  Connects everything together: menus, records, files and reports
// ============================================================================

class VehicleRentalSystem
{
private:
    vector<Vehicle> vehicles;
    vector<Customer> customers;
    vector<Rental> rentals;
    FileManager fileManager;
    ReportManager reportManager;

    // Search helpers (return an index, or -1 when not found)
    int findVehicleIndex(const string &vehicleId) const;
    int findCustomerIndex(const string &customerId) const;
    int findRentalIndex(const string &rentalId) const;
    int findVehicleByNumber(const string &vehicleNumber) const;
    int findCustomerByLicense(const string &licenseNumber) const;
    bool customerHasActiveRental(const string &customerId) const;
    bool vehicleHasActiveRental(const string &vehicleId) const;
    string generateRentalId() const;

    // Display helpers
    void displayVehicleTable(const vector<Vehicle> &list) const;
    void displayCustomerTable(const vector<Customer> &list) const;
    void displayRentalTable(const vector<Rental> &list) const;

    // Menus
    void vehicleMenu();
    void customerMenu();
    void rentalMenu();
    void searchMenu();
    void historyMenu();
    void reportMenu();
    void sortVehicleMenu();

    // Vehicle operations
    void addVehicle();
    void viewAllVehicles();
    void searchVehicle();
    void updateVehicle();
    void deleteVehicle();
    void viewAvailableVehicles();
    void viewRentedVehicles();

    // Customer operations
    void addCustomer();
    void viewAllCustomers();
    void searchCustomer();
    void updateCustomer();
    void deleteCustomer();
    void sortCustomers();
    void customerRentalHistory();

    // Rental operations
    void rentVehicle();
    void viewActiveRentals();
    void viewAllRentals();
    void returnVehicle();
    void cancelRental();
    void updatePaymentStatus();
    void sortRentals();

    // Search operations
    void searchRental();

    // History operations
    void viewHistoryByCustomer();
    void viewHistoryByVehicle();
    void viewHistoryByStatus(const string &status);

public:
    VehicleRentalSystem();
    void displayTitle() const;
    void run();
};

// ---------- Comparators used for sorting (called by std::sort) ----------

static bool compareVehicleById(const Vehicle &a, const Vehicle &b)
{
    return a.getVehicleId() < b.getVehicleId();
}

static bool compareVehicleByBrand(const Vehicle &a, const Vehicle &b)
{
    if (a.getBrand() == b.getBrand())
        return a.getVehicleId() < b.getVehicleId();
    return a.getBrand() < b.getBrand();
}

static bool compareVehicleByRate(const Vehicle &a, const Vehicle &b)
{
    return a.getDailyRate() < b.getDailyRate();
}

static bool compareVehicleByType(const Vehicle &a, const Vehicle &b)
{
    if (a.getType() == b.getType())
        return a.getVehicleId() < b.getVehicleId();
    return a.getType() < b.getType();
}

static bool compareCustomerByName(const Customer &a, const Customer &b)
{
    if (a.getName() == b.getName())
        return a.getCustomerId() < b.getCustomerId();
    return a.getName() < b.getName();
}

static bool compareRentalByDate(const Rental &a, const Rental &b)
{
    if (a.getRentalDate() == b.getRentalDate())
        return a.getRentalId() < b.getRentalId();
    return a.getRentalDate() < b.getRentalDate();
}

// ============================================================================
//  MAIN SYSTEM IMPLEMENTATION
// ============================================================================

// Constructor loads all stored records from the text files
VehicleRentalSystem::VehicleRentalSystem()
{
    fileManager.loadVehicles(vehicles);
    fileManager.loadCustomers(customers);
    fileManager.loadRentals(rentals);
}

void VehicleRentalSystem::displayTitle() const
{
    cout << "===============================================" << endl;
    cout << "     VEHICLE RENTAL MANAGEMENT SYSTEM" << endl;
    cout << "===============================================" << endl;
    cout << "Second Semester MIS Project" << endl;
    cout << endl;
    cout << "Prepared By : Nimolas Shrestha" << endl;
    cout << "Program     : BIT" << endl;
    cout << "Semester    : Second Semester" << endl;
    cout << "===============================================" << endl;
}

void VehicleRentalSystem::run()
{
    bool running = true;

    while (running)
    {
        cout << endl;
        cout << "==================================================" << endl;
        cout << "               MAIN MENU" << endl;
        cout << "==================================================" << endl;
        cout << "1. Vehicle Management" << endl;
        cout << "2. Customer Management" << endl;
        cout << "3. Rental Management" << endl;
        cout << "4. Search" << endl;
        cout << "5. Rental History" << endl;
        cout << "6. Reports" << endl;
        cout << "7. Exit" << endl;
        cout << "==================================================" << endl;

        int choice = readMenuChoice(7);

        switch (choice)
        {
        case 1:
            vehicleMenu();
            break;
        case 2:
            customerMenu();
            break;
        case 3:
            rentalMenu();
            break;
        case 4:
            searchMenu();
            break;
        case 5:
            historyMenu();
            break;
        case 6:
            reportMenu();
            break;
        case 7:
            cout << "Thank you for using the system. Goodbye!" << endl;
            running = false;
            break;
        }
    }
}

// ---------- Search / find helper functions ----------

int VehicleRentalSystem::findVehicleIndex(const string &vehicleId) const
{
    for (int i = 0; i < (int)vehicles.size(); i++)
    {
        if (vehicles[i].getVehicleId() == vehicleId)
            return i;
    }
    return -1;
}

int VehicleRentalSystem::findCustomerIndex(const string &customerId) const
{
    for (int i = 0; i < (int)customers.size(); i++)
    {
        if (customers[i].getCustomerId() == customerId)
            return i;
    }
    return -1;
}

int VehicleRentalSystem::findRentalIndex(const string &rentalId) const
{
    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].getRentalId() == rentalId)
            return i;
    }
    return -1;
}

int VehicleRentalSystem::findVehicleByNumber(const string &vehicleNumber) const
{
    for (int i = 0; i < (int)vehicles.size(); i++)
    {
        if (vehicles[i].getVehicleNumber() == vehicleNumber)
            return i;
    }
    return -1;
}

int VehicleRentalSystem::findCustomerByLicense(const string &licenseNumber) const
{
    for (int i = 0; i < (int)customers.size(); i++)
    {
        if (customers[i].getLicenseNumber() == licenseNumber)
            return i;
    }
    return -1;
}

bool VehicleRentalSystem::customerHasActiveRental(const string &customerId) const
{
    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].getCustomerId() == customerId && rentals[i].isActive())
            return true;
    }
    return false;
}

bool VehicleRentalSystem::vehicleHasActiveRental(const string &vehicleId) const
{
    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].getVehicleId() == vehicleId && rentals[i].isActive())
            return true;
    }
    return false;
}

// Generates the next rental ID like R001, R002 ... based on existing records
string VehicleRentalSystem::generateRentalId() const
{
    int maxNum = 0;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        string id = rentals[i].getRentalId();
        if (id.length() >= 2 && (id[0] == 'R' || id[0] == 'r'))
        {
            string numberPart = id.substr(1);
            bool allDigits = true;
            for (int j = 0; j < (int)numberPart.length(); j++)
            {
                if (!isdigit((unsigned char)numberPart[j]))
                    allDigits = false;
            }
            if (allDigits)
            {
                stringstream ss(numberPart);
                int num;
                ss >> num;
                if (num > maxNum)
                    maxNum = num;
            }
        }
    }

    stringstream ss;
    ss << "R" << setw(3) << setfill('0') << (maxNum + 1);
    return ss.str();
}

// ---------- Display helper functions ----------

void VehicleRentalSystem::displayVehicleTable(const vector<Vehicle> &list) const
{
    Vehicle::displayHeader();
    for (int i = 0; i < (int)list.size(); i++)
        list[i].display();
    cout << string(80, '-') << endl;
}

void VehicleRentalSystem::displayCustomerTable(const vector<Customer> &list) const
{
    Customer::displayHeader();
    for (int i = 0; i < (int)list.size(); i++)
        list[i].display();
    cout << string(95, '-') << endl;
}

void VehicleRentalSystem::displayRentalTable(const vector<Rental> &list) const
{
    Rental::displayHeader();
    for (int i = 0; i < (int)list.size(); i++)
        list[i].display();
    cout << string(115, '-') << endl;
}

// ---------- Main menus ----------

void VehicleRentalSystem::vehicleMenu()
{
    bool back = false;

    while (!back)
    {
        cout << endl;
        cout << "      VEHICLE MANAGEMENT" << endl;
        cout << "----------------------------------" << endl;
        cout << "1. Add Vehicle" << endl;
        cout << "2. View All Vehicles" << endl;
        cout << "3. Search Vehicle" << endl;
        cout << "4. Update Vehicle" << endl;
        cout << "5. Delete Vehicle" << endl;
        cout << "6. Sort Vehicles" << endl;
        cout << "7. View Available Vehicles" << endl;
        cout << "8. View Rented Vehicles" << endl;
        cout << "9. Back" << endl;
        cout << "----------------------------------" << endl;

        int choice = readMenuChoice(9);

        switch (choice)
        {
        case 1: addVehicle(); break;
        case 2: viewAllVehicles(); break;
        case 3: searchVehicle(); break;
        case 4: updateVehicle(); break;
        case 5: deleteVehicle(); break;
        case 6: sortVehicleMenu(); break;
        case 7: viewAvailableVehicles(); break;
        case 8: viewRentedVehicles(); break;
        case 9: back = true; break;
        }
    }
}

void VehicleRentalSystem::customerMenu()
{
    bool back = false;

    while (!back)
    {
        cout << endl;
        cout << "      CUSTOMER MANAGEMENT" << endl;
        cout << "----------------------------------" << endl;
        cout << "1. Add Customer" << endl;
        cout << "2. View All Customers" << endl;
        cout << "3. Search Customer" << endl;
        cout << "4. Update Customer" << endl;
        cout << "5. Delete Customer" << endl;
        cout << "6. Sort Customers by Name" << endl;
        cout << "7. Customer Rental History" << endl;
        cout << "8. Back" << endl;
        cout << "----------------------------------" << endl;

        int choice = readMenuChoice(8);

        switch (choice)
        {
        case 1: addCustomer(); break;
        case 2: viewAllCustomers(); break;
        case 3: searchCustomer(); break;
        case 4: updateCustomer(); break;
        case 5: deleteCustomer(); break;
        case 6: sortCustomers(); break;
        case 7: customerRentalHistory(); break;
        case 8: back = true; break;
        }
    }
}

void VehicleRentalSystem::rentalMenu()
{
    bool back = false;

    while (!back)
    {
        cout << endl;
        cout << "       RENTAL MANAGEMENT" << endl;
        cout << "----------------------------------" << endl;
        cout << "1. Rent Vehicle" << endl;
        cout << "2. View Active Rentals" << endl;
        cout << "3. View All Rentals" << endl;
        cout << "4. Return Vehicle" << endl;
        cout << "5. Cancel Rental" << endl;
        cout << "6. Payment Status" << endl;
        cout << "7. Sort Rentals by Date" << endl;
        cout << "8. Back" << endl;
        cout << "----------------------------------" << endl;

        int choice = readMenuChoice(8);

        switch (choice)
        {
        case 1: rentVehicle(); break;
        case 2: viewActiveRentals(); break;
        case 3: viewAllRentals(); break;
        case 4: returnVehicle(); break;
        case 5: cancelRental(); break;
        case 6: updatePaymentStatus(); break;
        case 7: sortRentals(); break;
        case 8: back = true; break;
        }
    }
}

void VehicleRentalSystem::searchMenu()
{
    bool back = false;

    while (!back)
    {
        cout << endl;
        cout << "             SEARCH" << endl;
        cout << "----------------------------------" << endl;
        cout << "1. Search Vehicle" << endl;
        cout << "2. Search Customer" << endl;
        cout << "3. Search Rental" << endl;
        cout << "4. Available Vehicles" << endl;
        cout << "5. Active Rentals" << endl;
        cout << "6. Completed Rentals" << endl;
        cout << "7. Back" << endl;
        cout << "----------------------------------" << endl;

        int choice = readMenuChoice(7);

        switch (choice)
        {
        case 1: searchVehicle(); break;
        case 2: searchCustomer(); break;
        case 3: searchRental(); break;
        case 4: viewAvailableVehicles(); break;
        case 5: viewHistoryByStatus("Active"); break;
        case 6: viewHistoryByStatus("Completed"); break;
        case 7: back = true; break;
        }
    }
}

void VehicleRentalSystem::historyMenu()
{
    bool back = false;

    while (!back)
    {
        cout << endl;
        cout << "         RENTAL HISTORY" << endl;
        cout << "----------------------------------" << endl;
        cout << "1. All Rental History" << endl;
        cout << "2. Customer-wise History" << endl;
        cout << "3. Vehicle-wise History" << endl;
        cout << "4. Active Rentals" << endl;
        cout << "5. Completed Rentals" << endl;
        cout << "6. Cancelled Rentals" << endl;
        cout << "7. Back" << endl;
        cout << "----------------------------------" << endl;

        int choice = readMenuChoice(7);

        switch (choice)
        {
        case 1: viewAllRentals(); break;
        case 2: viewHistoryByCustomer(); break;
        case 3: viewHistoryByVehicle(); break;
        case 4: viewHistoryByStatus("Active"); break;
        case 5: viewHistoryByStatus("Completed"); break;
        case 6: viewHistoryByStatus("Cancelled"); break;
        case 7: back = true; break;
        }
    }
}

void VehicleRentalSystem::reportMenu()
{
    bool back = false;

    while (!back)
    {
        cout << endl;
        cout << "             REPORTS" << endl;
        cout << "----------------------------------" << endl;
        cout << "1. Vehicle Summary" << endl;
        cout << "2. Customer Summary" << endl;
        cout << "3. Rental Summary" << endl;
        cout << "4. Income Summary" << endl;
        cout << "5. Fine Summary" << endl;
        cout << "6. Outstanding Payment" << endl;
        cout << "7. Complete System Summary" << endl;
        cout << "8. Back" << endl;
        cout << "----------------------------------" << endl;

        int choice = readMenuChoice(8);

        switch (choice)
        {
        case 1: reportManager.vehicleSummary(vehicles); break;
        case 2: reportManager.customerSummary(customers, rentals); break;
        case 3: reportManager.rentalSummary(rentals); break;
        case 4: reportManager.incomeSummary(rentals); break;
        case 5: reportManager.fineSummary(rentals); break;
        case 6: reportManager.outstandingPayments(rentals, customers); break;
        case 7: reportManager.completeSummary(vehicles, customers, rentals); break;
        case 8: back = true; break;
        }

        if (choice >= 1 && choice <= 7)
            pauseConsole();
    }
}

void VehicleRentalSystem::sortVehicleMenu()
{
    bool back = false;

    while (!back)
    {
        cout << endl;
        cout << "         SORT VEHICLES" << endl;
        cout << "----------------------------------" << endl;
        cout << "1. Sort by Vehicle ID" << endl;
        cout << "2. Sort by Brand" << endl;
        cout << "3. Sort by Daily Rate" << endl;
        cout << "4. Sort by Type" << endl;
        cout << "5. Back" << endl;
        cout << "----------------------------------" << endl;

        int choice = readMenuChoice(5);

        switch (choice)
        {
        case 1:
            sort(vehicles.begin(), vehicles.end(), compareVehicleById);
            cout << "Vehicles sorted by ID." << endl;
            viewAllVehicles();
            break;
        case 2:
            sort(vehicles.begin(), vehicles.end(), compareVehicleByBrand);
            cout << "Vehicles sorted by Brand." << endl;
            viewAllVehicles();
            break;
        case 3:
            sort(vehicles.begin(), vehicles.end(), compareVehicleByRate);
            cout << "Vehicles sorted by Daily Rate." << endl;
            viewAllVehicles();
            break;
        case 4:
            sort(vehicles.begin(), vehicles.end(), compareVehicleByType);
            cout << "Vehicles sorted by Type." << endl;
            viewAllVehicles();
            break;
        case 5:
            back = true;
            break;
        }
    }
}

// ---------- Vehicle operations ----------

void VehicleRentalSystem::addVehicle()
{
    cout << endl << "         ADD VEHICLE" << endl;
    cout << "----------------------------------" << endl;

    Vehicle v;
    v.input();

    if (findVehicleIndex(v.getVehicleId()) >= 0)
    {
        cout << "Vehicle ID already exists." << endl;
        pauseConsole();
        return;
    }

    if (findVehicleByNumber(v.getVehicleNumber()) >= 0)
    {
        cout << "Vehicle number already exists." << endl;
        pauseConsole();
        return;
    }

    vehicles.push_back(v);
    fileManager.saveVehicles(vehicles);

    cout << "Vehicle added successfully." << endl;
    cout << "Vehicle ID: " << v.getVehicleId() << endl;
    pauseConsole();
}

void VehicleRentalSystem::viewAllVehicles()
{
    cout << endl << "         ALL VEHICLES" << endl;
    cout << "----------------------------------" << endl;

    if (vehicles.empty())
    {
        cout << "No vehicles found." << endl;
    }
    else
    {
        displayVehicleTable(vehicles);
    }
    pauseConsole();
}

void VehicleRentalSystem::searchVehicle()
{
    cout << endl << "         SEARCH VEHICLE" << endl;
    cout << "----------------------------------" << endl;

    string keyword = readLineNonEmpty("Enter Vehicle ID / Number / Brand / Model / Type: ");
    string key = toLower(keyword);

    vector<Vehicle> results;

    for (int i = 0; i < (int)vehicles.size(); i++)
    {
        Vehicle v = vehicles[i];
        if (toLower(v.getVehicleId()).find(key) != string::npos ||
            toLower(v.getVehicleNumber()).find(key) != string::npos ||
            toLower(v.getBrand()).find(key) != string::npos ||
            toLower(v.getModel()).find(key) != string::npos ||
            toLower(v.getType()).find(key) != string::npos)
        {
            results.push_back(v);
        }
    }

    if (results.empty())
    {
        cout << "No matching vehicles found." << endl;
    }
    else
    {
        cout << results.size() << " matching vehicle(s) found:" << endl;
        displayVehicleTable(results);
    }
    pauseConsole();
}

void VehicleRentalSystem::updateVehicle()
{
    cout << endl << "         UPDATE VEHICLE" << endl;
    cout << "----------------------------------" << endl;

    string id = readLineNonEmpty("Enter Vehicle ID to update: ");
    int index = findVehicleIndex(id);

    if (index < 0)
    {
        cout << "Vehicle not found." << endl;
        pauseConsole();
        return;
    }

    Vehicle &v = vehicles[index];

    cout << "Current record:" << endl;
    vector<Vehicle> oneRow;
    oneRow.push_back(v);
    displayVehicleTable(oneRow);

    string number = readLineNonEmpty("New Vehicle Number: ");
    int dupIndex = findVehicleByNumber(number);
    if (dupIndex >= 0 && dupIndex != index)
    {
        cout << "Vehicle number already exists." << endl;
        pauseConsole();
        return;
    }

    string brand = readLineNonEmpty("New Brand: ");
    string model = readLineNonEmpty("New Model: ");
    string type = readLineNonEmpty("New Type: ");
    int year = readInt("New Year: ", 1900, 2100);
    double rate = readDouble("New Daily Rental Rate: ", 0, "Rental rate cannot be negative.");

    v.setVehicleNumber(number);
    v.setBrand(brand);
    v.setModel(model);
    v.setType(type);
    v.setYear(year);
    v.setDailyRate(rate);

    fileManager.saveVehicles(vehicles);

    cout << "Vehicle updated successfully." << endl;
    pauseConsole();
}

void VehicleRentalSystem::deleteVehicle()
{
    cout << endl << "         DELETE VEHICLE" << endl;
    cout << "----------------------------------" << endl;

    string id = readLineNonEmpty("Enter Vehicle ID to delete: ");
    int index = findVehicleIndex(id);

    if (index < 0)
    {
        cout << "Vehicle not found." << endl;
        pauseConsole();
        return;
    }

    if (vehicleHasActiveRental(vehicles[index].getVehicleId()))
    {
        cout << "Cannot delete. Vehicle is currently rented." << endl;
        pauseConsole();
        return;
    }

    cout << "Vehicle to delete:" << endl;
    vector<Vehicle> oneRow;
    oneRow.push_back(vehicles[index]);
    displayVehicleTable(oneRow);

    if (!askConfirmation("Are you sure you want to delete this vehicle? (Y/N): "))
    {
        cout << "Deletion cancelled." << endl;
        pauseConsole();
        return;
    }

    vehicles.erase(vehicles.begin() + index);
    fileManager.saveVehicles(vehicles);

    cout << "Vehicle deleted successfully." << endl;
    pauseConsole();
}

void VehicleRentalSystem::viewAvailableVehicles()
{
    cout << endl << "      AVAILABLE VEHICLES" << endl;
    cout << "----------------------------------" << endl;

    vector<Vehicle> availableList;

    for (int i = 0; i < (int)vehicles.size(); i++)
    {
        if (vehicles[i].isAvailable())
            availableList.push_back(vehicles[i]);
    }

    if (availableList.empty())
    {
        cout << "No vehicles are currently available." << endl;
    }
    else
    {
        cout << availableList.size() << " vehicle(s) available:" << endl;
        displayVehicleTable(availableList);
    }
    pauseConsole();
}

void VehicleRentalSystem::viewRentedVehicles()
{
    cout << endl << "      RENTED VEHICLES" << endl;
    cout << "----------------------------------" << endl;

    vector<Vehicle> rentedList;

    for (int i = 0; i < (int)vehicles.size(); i++)
    {
        if (!vehicles[i].isAvailable())
            rentedList.push_back(vehicles[i]);
    }

    if (rentedList.empty())
    {
        cout << "No vehicles are currently rented." << endl;
    }
    else
    {
        cout << rentedList.size() << " vehicle(s) rented:" << endl;
        displayVehicleTable(rentedList);
    }
    pauseConsole();
}

// ---------- Customer operations ----------

void VehicleRentalSystem::addCustomer()
{
    cout << endl << "         ADD CUSTOMER" << endl;
    cout << "----------------------------------" << endl;

    Customer c;
    c.input();

    if (findCustomerIndex(c.getCustomerId()) >= 0)
    {
        cout << "Customer ID already exists." << endl;
        pauseConsole();
        return;
    }

    if (findCustomerByLicense(c.getLicenseNumber()) >= 0)
    {
        cout << "License number already exists." << endl;
        pauseConsole();
        return;
    }

    customers.push_back(c);
    fileManager.saveCustomers(customers);

    cout << "Customer added successfully." << endl;
    cout << "Customer ID: " << c.getCustomerId() << endl;
    pauseConsole();
}

void VehicleRentalSystem::viewAllCustomers()
{
    cout << endl << "         ALL CUSTOMERS" << endl;
    cout << "----------------------------------" << endl;

    if (customers.empty())
    {
        cout << "No customers found." << endl;
    }
    else
    {
        displayCustomerTable(customers);
    }
    pauseConsole();
}

void VehicleRentalSystem::searchCustomer()
{
    cout << endl << "         SEARCH CUSTOMER" << endl;
    cout << "----------------------------------" << endl;

    string keyword = readLineNonEmpty("Enter Customer ID / Name / Phone / License: ");
    string key = toLower(keyword);

    vector<Customer> results;

    for (int i = 0; i < (int)customers.size(); i++)
    {
        Customer c = customers[i];
        if (toLower(c.getCustomerId()).find(key) != string::npos ||
            toLower(c.getName()).find(key) != string::npos ||
            toLower(c.getPhone()).find(key) != string::npos ||
            toLower(c.getLicenseNumber()).find(key) != string::npos)
        {
            results.push_back(c);
        }
    }

    if (results.empty())
    {
        cout << "No matching customers found." << endl;
    }
    else
    {
        cout << results.size() << " matching customer(s) found:" << endl;
        displayCustomerTable(results);
    }
    pauseConsole();
}

void VehicleRentalSystem::updateCustomer()
{
    cout << endl << "         UPDATE CUSTOMER" << endl;
    cout << "----------------------------------" << endl;

    string id = readLineNonEmpty("Enter Customer ID to update: ");
    int index = findCustomerIndex(id);

    if (index < 0)
    {
        cout << "Customer not found." << endl;
        pauseConsole();
        return;
    }

    Customer &c = customers[index];

    cout << "Current record:" << endl;
    vector<Customer> oneRow;
    oneRow.push_back(c);
    displayCustomerTable(oneRow);

    string name = readLineNonEmpty("New Name: ");
    string phone = readLineNonEmpty("New Phone: ");
    string email = readLineNonEmpty("New Email: ");
    string address = readLineNonEmpty("New Address: ");

    string license = readLineNonEmpty("New Driving License Number: ");
    int dupIndex = findCustomerByLicense(license);
    if (dupIndex >= 0 && dupIndex != index)
    {
        cout << "License number already exists." << endl;
        pauseConsole();
        return;
    }

    c.setName(name);
    c.setPhone(phone);
    c.setEmail(email);
    c.setAddress(address);
    c.setLicenseNumber(license);

    fileManager.saveCustomers(customers);

    cout << "Customer updated successfully." << endl;
    pauseConsole();
}

void VehicleRentalSystem::deleteCustomer()
{
    cout << endl << "         DELETE CUSTOMER" << endl;
    cout << "----------------------------------" << endl;

    string id = readLineNonEmpty("Enter Customer ID to delete: ");
    int index = findCustomerIndex(id);

    if (index < 0)
    {
        cout << "Customer not found." << endl;
        pauseConsole();
        return;
    }

    if (customerHasActiveRental(customers[index].getCustomerId()))
    {
        cout << "Cannot delete. Customer has an active rental." << endl;
        pauseConsole();
        return;
    }

    cout << "Customer to delete:" << endl;
    vector<Customer> oneRow;
    oneRow.push_back(customers[index]);
    displayCustomerTable(oneRow);

    if (!askConfirmation("Are you sure you want to delete this customer? (Y/N): "))
    {
        cout << "Deletion cancelled." << endl;
        pauseConsole();
        return;
    }

    customers.erase(customers.begin() + index);
    fileManager.saveCustomers(customers);

    cout << "Customer deleted successfully." << endl;
    pauseConsole();
}

void VehicleRentalSystem::sortCustomers()
{
    sort(customers.begin(), customers.end(), compareCustomerByName);
    cout << "Customers sorted by Name." << endl;
    viewAllCustomers();
}

void VehicleRentalSystem::customerRentalHistory()
{
    cout << endl << "     CUSTOMER RENTAL HISTORY" << endl;
    cout << "----------------------------------" << endl;

    string id = readLineNonEmpty("Enter Customer ID: ");

    if (findCustomerIndex(id) < 0)
    {
        cout << "Customer not found." << endl;
        pauseConsole();
        return;
    }

    vector<Rental> history;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].getCustomerId() == id)
            history.push_back(rentals[i]);
    }

    if (history.empty())
    {
        cout << "No rental history found for this customer." << endl;
    }
    else
    {
        cout << "Rental history for customer " << id << ":" << endl;
        displayRentalTable(history);
    }
    pauseConsole();
}

// ---------- Rental operations ----------

void VehicleRentalSystem::rentVehicle()
{
    cout << endl << "         RENT VEHICLE" << endl;
    cout << "----------------------------------" << endl;

    string customerId = readLineNonEmpty("Enter Customer ID: ");
    int cIndex = findCustomerIndex(customerId);

    if (cIndex < 0)
    {
        cout << "Customer not found." << endl;
        pauseConsole();
        return;
    }

    string vehicleId = readLineNonEmpty("Enter Vehicle ID: ");
    int vIndex = findVehicleIndex(vehicleId);

    if (vIndex < 0)
    {
        cout << "Vehicle not found." << endl;
        pauseConsole();
        return;
    }

    if (!vehicles[vIndex].isAvailable())
    {
        cout << "Vehicle is currently unavailable." << endl;
        pauseConsole();
        return;
    }

    string rentalDate = readLineNonEmpty("Enter Rental Date (YYYY-MM-DD): ");
    if (!isValidDate(rentalDate))
    {
        cout << "Invalid date. Please use format YYYY-MM-DD." << endl;
        pauseConsole();
        return;
    }

    string expectedDate = readLineNonEmpty("Enter Expected Return Date (YYYY-MM-DD): ");
    if (!isValidDate(expectedDate))
    {
        cout << "Invalid date. Please use format YYYY-MM-DD." << endl;
        pauseConsole();
        return;
    }

    int days = daysBetween(rentalDate, expectedDate);
    if (days < 1)
    {
        cout << "Expected return date must be after rental date." << endl;
        pauseConsole();
        return;
    }

    double charge = days * vehicles[vIndex].getDailyRate();

    // Rental summary shown before the user confirms
    cout << endl;
    cout << "         RENTAL SUMMARY" << endl;
    cout << "----------------------------------" << endl;
    cout << "Customer      : " << customers[cIndex].getCustomerId()
         << " - " << customers[cIndex].getName() << endl;
    cout << "Vehicle       : " << vehicles[vIndex].getVehicleId()
         << " - " << vehicles[vIndex].getBrand() << " " << vehicles[vIndex].getModel() << endl;
    cout << "Vehicle Number: " << vehicles[vIndex].getVehicleNumber() << endl;
    cout << "Daily Rate    : Rs. " << vehicles[vIndex].getDailyRate() << endl;
    cout << "Rental Date   : " << rentalDate << endl;
    cout << "Expected Return: " << expectedDate << endl;
    cout << "Rental Days   : " << days << endl;
    cout << "Rental Charge : Rs. " << charge << endl;
    cout << "----------------------------------" << endl;

    if (!askConfirmation("Confirm rental? (Y/N): "))
    {
        cout << "Rental cancelled by user." << endl;
        pauseConsole();
        return;
    }

    string rentalId = generateRentalId();

    Rental newRental(rentalId, customerId, vehicleId, rentalDate, expectedDate,
                     "-", days, charge, 0, charge, "Pending", "Active");

    rentals.push_back(newRental);
    vehicles[vIndex].setAvailable(false);

    fileManager.saveRentals(rentals);
    fileManager.saveVehicles(vehicles);

    cout << endl << "Vehicle rented successfully!" << endl;
    cout << "Rental ID: " << rentalId << endl;
    pauseConsole();
}

void VehicleRentalSystem::viewActiveRentals()
{
    cout << endl << "         ACTIVE RENTALS" << endl;
    cout << "----------------------------------" << endl;

    vector<Rental> activeList;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].isActive())
            activeList.push_back(rentals[i]);
    }

    if (activeList.empty())
    {
        cout << "No active rentals." << endl;
    }
    else
    {
        displayRentalTable(activeList);
    }
    pauseConsole();
}

void VehicleRentalSystem::viewAllRentals()
{
    cout << endl << "         ALL RENTALS" << endl;
    cout << "----------------------------------" << endl;

    if (rentals.empty())
    {
        cout << "No rental records found." << endl;
    }
    else
    {
        displayRentalTable(rentals);
    }
    pauseConsole();
}

void VehicleRentalSystem::returnVehicle()
{
    cout << endl << "         RETURN VEHICLE" << endl;
    cout << "----------------------------------" << endl;

    string rentalId = readLineNonEmpty("Enter Rental ID: ");
    int rIndex = findRentalIndex(rentalId);

    if (rIndex < 0)
    {
        cout << "Rental record not found." << endl;
        pauseConsole();
        return;
    }

    Rental &r = rentals[rIndex];

    if (r.isCancelled())
    {
        cout << "This rental was cancelled. It cannot be returned." << endl;
        pauseConsole();
        return;
    }

    if (!r.isActive())
    {
        cout << "This rental has already been completed." << endl;
        pauseConsole();
        return;
    }

    string actualDate = readLineNonEmpty("Enter Actual Return Date (YYYY-MM-DD): ");
    if (!isValidDate(actualDate))
    {
        cout << "Invalid date. Please use format YYYY-MM-DD." << endl;
        pauseConsole();
        return;
    }

    if (daysBetween(r.getRentalDate(), actualDate) < 0)
    {
        cout << "Invalid return date. Return date cannot be before rental date." << endl;
        pauseConsole();
        return;
    }

    // Late days and fine calculation
    int lateDays = daysBetween(r.getExpectedReturnDate(), actualDate);
    if (lateDays < 0)
        lateDays = 0;

    double fine = lateDays * LATE_FINE_PER_DAY;
    double total = r.getRentalCharge() + fine;

    cout << endl;
    cout << "         PAYMENT SUMMARY" << endl;
    cout << "----------------------------------" << endl;
    cout << left << setw(17) << "Rental Charge" << ": Rs. " << r.getRentalCharge() << endl;
    cout << left << setw(17) << "Late Fine" << ": Rs. " << fine << endl;
    cout << left << setw(17) << "Total Amount" << ": Rs. " << total << endl;
    cout << left << setw(17) << "Late Days" << ": " << lateDays << endl;
    cout << left << setw(17) << "Payment" << ": Pending" << endl;
    cout << "----------------------------------" << endl;

    if (!askConfirmation("Confirm return? (Y/N): "))
    {
        cout << "Return cancelled by user." << endl;
        pauseConsole();
        return;
    }

    r.setActualReturnDate(actualDate);
    r.setFine(fine);
    r.setTotalAmount(total);
    r.setRentalStatus("Completed");

    int vIndex = findVehicleIndex(r.getVehicleId());
    if (vIndex >= 0)
        vehicles[vIndex].setAvailable(true);

    fileManager.saveRentals(rentals);
    fileManager.saveVehicles(vehicles);

    cout << endl << "Vehicle returned successfully." << endl;
    pauseConsole();
}

void VehicleRentalSystem::cancelRental()
{
    cout << endl << "         CANCEL RENTAL" << endl;
    cout << "----------------------------------" << endl;

    string rentalId = readLineNonEmpty("Enter Rental ID: ");
    int rIndex = findRentalIndex(rentalId);

    if (rIndex < 0)
    {
        cout << "Rental record not found." << endl;
        pauseConsole();
        return;
    }

    Rental &r = rentals[rIndex];

    if (!r.isActive())
    {
        cout << "Only active rentals can be cancelled." << endl;
        pauseConsole();
        return;
    }

    cout << "Rental to cancel:" << endl;
    vector<Rental> oneRow;
    oneRow.push_back(r);
    displayRentalTable(oneRow);

    if (!askConfirmation("Are you sure you want to cancel this rental? (Y/N): "))
    {
        cout << "Cancellation stopped." << endl;
        pauseConsole();
        return;
    }

    r.setRentalDays(0);
    r.setRentalCharge(0);
    r.setFine(0);
    r.setTotalAmount(0);
    r.setPaymentStatus("None");
    r.setRentalStatus("Cancelled");

    int vIndex = findVehicleIndex(r.getVehicleId());
    if (vIndex >= 0)
        vehicles[vIndex].setAvailable(true);

    fileManager.saveRentals(rentals);
    fileManager.saveVehicles(vehicles);

    cout << "Rental cancelled successfully." << endl;
    pauseConsole();
}

void VehicleRentalSystem::updatePaymentStatus()
{
    cout << endl << "         PAYMENT STATUS" << endl;
    cout << "----------------------------------" << endl;

    if (rentals.empty())
    {
        cout << "No rental records found." << endl;
        pauseConsole();
        return;
    }

    displayRentalTable(rentals);

    string rentalId = readLineNonEmpty("Enter Rental ID to mark as Paid (or 0 to go back): ");

    if (rentalId == "0")
        return;

    int rIndex = findRentalIndex(rentalId);

    if (rIndex < 0)
    {
        cout << "Rental record not found." << endl;
        pauseConsole();
        return;
    }

    Rental &r = rentals[rIndex];

    if (r.isCancelled())
    {
        cout << "Cancelled rentals have no payment." << endl;
        pauseConsole();
        return;
    }

    if (r.getPaymentStatus() == "Paid")
    {
        cout << "This rental is already paid." << endl;
        pauseConsole();
        return;
    }

    if (r.isActive())
    {
        cout << "This rental has not been completed yet. Payment cannot be marked." << endl;
        pauseConsole();
        return;
    }

    r.setPaymentStatus("Paid");
    fileManager.saveRentals(rentals);

    cout << "Payment for " << rentalId << " marked as Paid." << endl;
    cout << "Amount: Rs. " << r.getTotalAmount() << endl;
    pauseConsole();
}

void VehicleRentalSystem::sortRentals()
{
    sort(rentals.begin(), rentals.end(), compareRentalByDate);
    cout << "Rentals sorted by Rental Date." << endl;
    viewAllRentals();
}

// ---------- Search / history operations ----------

void VehicleRentalSystem::searchRental()
{
    cout << endl << "         SEARCH RENTAL" << endl;
    cout << "----------------------------------" << endl;

    string keyword = readLineNonEmpty("Enter Rental ID / Customer ID / Vehicle ID / Status: ");
    string key = toLower(keyword);

    vector<Rental> results;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        Rental r = rentals[i];
        if (toLower(r.getRentalId()).find(key) != string::npos ||
            toLower(r.getCustomerId()).find(key) != string::npos ||
            toLower(r.getVehicleId()).find(key) != string::npos ||
            toLower(r.getRentalStatus()).find(key) != string::npos ||
            toLower(r.getPaymentStatus()).find(key) != string::npos)
        {
            results.push_back(r);
        }
    }

    if (results.empty())
    {
        cout << "No matching rentals found." << endl;
    }
    else
    {
        cout << results.size() << " matching rental(s) found:" << endl;
        displayRentalTable(results);
    }
    pauseConsole();
}

void VehicleRentalSystem::viewHistoryByCustomer()
{
    cout << endl << "      CUSTOMER-WISE HISTORY" << endl;
    cout << "----------------------------------" << endl;

    string customerId = readLineNonEmpty("Enter Customer ID: ");

    if (findCustomerIndex(customerId) < 0)
    {
        cout << "Customer not found." << endl;
        pauseConsole();
        return;
    }

    vector<Rental> history;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].getCustomerId() == customerId)
            history.push_back(rentals[i]);
    }

    if (history.empty())
    {
        cout << "No rental history for this customer." << endl;
    }
    else
    {
        displayRentalTable(history);
    }
    pauseConsole();
}

void VehicleRentalSystem::viewHistoryByVehicle()
{
    cout << endl << "      VEHICLE-WISE HISTORY" << endl;
    cout << "----------------------------------" << endl;

    string vehicleId = readLineNonEmpty("Enter Vehicle ID: ");

    if (findVehicleIndex(vehicleId) < 0)
    {
        cout << "Vehicle not found." << endl;
        pauseConsole();
        return;
    }

    vector<Rental> history;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].getVehicleId() == vehicleId)
            history.push_back(rentals[i]);
    }

    if (history.empty())
    {
        cout << "No rental history for this vehicle." << endl;
    }
    else
    {
        displayRentalTable(history);
    }
    pauseConsole();
}

void VehicleRentalSystem::viewHistoryByStatus(const string &status)
{
    cout << endl << "      " << status << " RENTALS" << endl;
    cout << "----------------------------------" << endl;

    vector<Rental> result;

    for (int i = 0; i < (int)rentals.size(); i++)
    {
        if (rentals[i].getRentalStatus() == status)
            result.push_back(rentals[i]);
    }

    if (result.empty())
    {
        cout << "No " << status << " rentals found." << endl;
    }
    else
    {
        displayRentalTable(result);
    }
    pauseConsole();
}

// ============================================================================
//  MAIN FUNCTION
// ============================================================================

int main()
{
    VehicleRentalSystem system;

    system.displayTitle();
    system.run();

    return 0;
}
