#! /usr/bin/python3
import os
import argparse
import sys
import subprocess
import shutil
import multiprocessing

VERSION = sys.version_info.major + (sys.version_info.minor / 10)

# System specific settings

TEST = [os.path.join('build','unittest')]

def subprocess_run(args):
    if(VERSION < 3.5):
        return subprocess.call(args)
    else:
        return subprocess.run(args).returncode

if __name__ == '__main__':
    result = 0
    try:
        result = subprocess_run(TEST)
    except:
        result = 1
        print(sys.last_traceback)
    sys.exit(result)