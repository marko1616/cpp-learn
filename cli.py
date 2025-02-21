import io
import argparse
import zipfile
import subprocess
import platform
import requests
import shutil

from pathlib import Path

platform_type = platform.system()
package_managers = ["apt", "yum", "pacman"]
npcap_sdk_url = "https://npcap.com/dist/npcap-sdk-1.15.zip"

def download_file_to_memory(url):
    response = requests.get(url, stream=True)
    if response.status_code == 200:
        file_in_memory = io.BytesIO()
        for chunk in response.iter_content(chunk_size=8192):
            file_in_memory.write(chunk)
        file_in_memory.seek(0)
        print(f"{url} downloaded successfully.")
        return file_in_memory
    else:
        print(f"Failed to download {url}")
        return None

def yes_no_prompt(prompt):
    while True:
        user_input = input(prompt).strip().lower()
        if user_input == 'y':
            return True
        elif user_input == 'n':
            return False
        else:
            print("Invalid input. Please enter 'y' or 'n'.")

def lint():
    src_path = Path(__file__).parent / "src"
    cpp_files = list(src_path.glob("**/*.cpp")) + list(src_path.glob("**/*.h"))
    
    for file in cpp_files:
        subprocess.run(["cppcheck", "--enable=all", "--suppress=missingInclude", "--suppress=missingIncludeSystem", str(file)])
    
    print("Linting completed.")

def init():
    subprocess.run(["git", "submodule", "update", "--init", "--recursive"])
    if platform_type == "Windows":
        if yes_no_prompt("Do you want to install Npcap SDK? (y/n)"):
            file_in_memory = download_file_to_memory(npcap_sdk_url)
            if file_in_memory:
                with zipfile.ZipFile(file_in_memory, 'r') as zip_ref:
                    zip_ref.extractall(Path(__file__).parent / "libs/npcap-sdk")
        else:
            print("You will need to install Npcap SDK manually.")
        subprocess.run(["cmake", "-DPCAP_ROOT=./libs/npcap-sdk/", "-S", ".", "-B","build"])
    elif platform_type == "Linux":
        system_package_manager = None
        for package_manager in package_managers:
            if shutil.which(package_manager):
                system_package_manager = package_manager
                break
        if system_package_manager == "apt" and yes_no_prompt("Do you want to install libpcap-dev using apt? (y/n)"):
            subprocess.run(["sudo", "apt", "update"])
            subprocess.run(["sudo", "apt", "install", "libpcap-dev"])
        elif system_package_manager == "yum" and yes_no_prompt("Do you want to install libpcap-devel using yum? (y/n)"):
            subprocess.run(["sudo", "yum", "install", "libpcap-devel"])
        elif system_package_manager == "pacman" and yes_no_prompt("Do you want to install libpcap using pacman? (y/n)"):
            subprocess.run(["sudo", "pacman", "-S", "libpcap"])
        else:
            print("You will need to install libpcap-dev manually.")
        subprocess.run(["cmake", "-DPCAP_ROOT=/usr/local/", "-S", ".", "-B","build"])
    else:
        raise NotImplementedError("Unsupported platform")
    print("Initialization completed.")

def build():
    subprocess.run(["cmake", "--build", "build"])
    print("Build completed.")

def main():
    parser = argparse.ArgumentParser(description="CLI tool for project management")
    parser.add_argument('command', choices=['lint', 'init', 'build'], help='Command to execute')
    
    args = parser.parse_args()
    
    if args.command == 'lint':
        lint()
    elif args.command == 'init':
        init()
    elif args.command == 'build':
        build()

if __name__ == "__main__":
    main()
