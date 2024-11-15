import os
import time
import subprocess
from colorama import init, Fore, Style

# Initialize colorama
init(autoreset=True)

def check_device_exists(device_path):
    return os.path.exists(device_path)

def upload_firmware():
    result = subprocess.run(["platformio", "run", "--target", "upload", "--environment", "Master-Device"], capture_output=True, text=True)
    if result.returncode == 0:
        print(Fore.GREEN + "done")
    else:
        print(Fore.RED + "Upload failed")
        print(Fore.RED + result.stderr)

def main():
    device_path = "/dev/cu.usbmodem11301"
    while True:
        if check_device_exists(device_path):
            print(Fore.YELLOW + f"Device {device_path} connected. Uploading firmware...")
            upload_firmware()
            while check_device_exists(device_path):
                time.sleep(1)
            print(Fore.YELLOW + f"Device {device_path} disconnected. Waiting for new device...")
        time.sleep(1)

if __name__ == "__main__":
    main()