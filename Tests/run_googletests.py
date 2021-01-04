#!/bin/python

import sys
import os
import os.path
import glob
import subprocess

tests = [
    "testCmAvrGccCompiler.exe",
    "testAS7DeviceResolver.exe",
    "testCmStringUtils.exe"
]

# https://stackoverflow.com/questions/1724693/find-a-file-in-python
def find(name, path):
    for root, _ , files in os.walk(path):
        if name in files:
            return os.path.join(root, name)
    return None

def find_test_executables(build_dir_path):
    out = []
    abs_build = os.path.abspath(build_dir_path)
    if not os.path.exists(abs_build) :
        print("Build directory path does not exist : {}".format("build_dir_path"))
        print("Current working directory is : {}\n".format(os.getcwd()))
        print_help()
        exit(-1)

    # Append only built tests
    for test in tests:
        found_test = find(test, os.path.join(abs_build, "Tests"))
        if found_test is not None:
            out.append(found_test)

    return out

def print_help():
    print("This tool is used to run all googletests sequentially located within a build directory")
    print("Usage :")
    print("   python run_googletests.py <build directory>")
    print("      E.g. : \"python run_googletests.py ./build\"")
    print("\n")

class TestResult:
    def __init__(self, name):
        self.name = name
        self.success = True
        self.logs = ""

def main(args):
    executables = find_test_executables(args[1])
    test_results = []
    has_failures = False
    for exe in executables:
        test_result = TestResult(exe)
        print("Running test : {}".format(exe))
        process = subprocess.run(exe, stdout=subprocess.PIPE, universal_newlines=True)
        test_result.logs = process.stdout
        if process.returncode != 0 :
            test_result.success = False
            has_failures = True
        print(test_result.logs)
        test_results.append(test_result)

    if has_failures:
        print("Some tests might have failed, here is a list of failed tests :")
        for test in test_results :
            if test.success == False:
                print("- {}".format(test.name))
        exit(-1)
    exit(0)

if __name__ == "__main__" :
    main(sys.argv)

