#! /usr/bin/python3
import os
import argparse
import sys
import subprocess
import shutil
import multiprocessing
import time
import tempfile
def eprint(message):
        sys.stderr.write(message)
        sys.stderr.flush()

VERSION = sys.version_info.major + (sys.version_info.minor / 10)
TIMESTAMP = str(int(time.time()))
DATA = os.path.join(tempfile.gettempdir(),'rmp-' + TIMESTAMP)

# System specific settings

WIN = (os.name == 'nt')
TESTD = [
    os.path.join('build','server'),
    str(12345),
    DATA]
TEST = [os.path.join('build','unittest')]

def subprocess_run(args):
    if(VERSION < 3.5):
        return subprocess.call(args)
    else:
        return subprocess.run(args).returncode

if __name__ == '__main__':
    result = 0
    try:
        os.mkdir(DATA)
        server = subprocess.Popen(TESTD, stderr=subprocess.PIPE, shell=WIN)
        eprint('starting server...')
        time.sleep(1)
        client = subprocess.Popen(TEST, stderr=subprocess.PIPE, shell=WIN)
        client.wait()
        server.kill()
        shutil.rmtree(DATA,ignore_errors=True)
    except:
        result = 1
        eprint(sys.last_traceback)
    sys.exit(result)