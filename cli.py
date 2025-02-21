import argparse
import subprocess
from pathlib import Path

def lint():
    src_path = Path(__file__).parent / "src"
    cpp_files = list(src_path.glob("**/*.cpp")) + list(src_path.glob("**/*.h"))
    
    for file in cpp_files:
        subprocess.run(["cppcheck", "--enable=all", "--suppress=missingInclude", "--suppress=missingIncludeSystem", str(file)])
    
    print("Linting completed.")

def init():
    subprocess.run(["git", "submodule", "update", "--init", "--recursive"])
    subprocess.run(["cmake", "-DPCAP_ROOT=./libs/npcap-sdk/", "-S", ".", "-B", "build"])
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
