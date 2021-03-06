#! /usr/bin/python3
import os
import argparse
import sys
import subprocess
import shutil
import multiprocessing

def eprint(message):
        sys.stderr.write(str(message))
        sys.stderr.flush()

CORES = multiprocessing.cpu_count()

# System specific settings

if(os.name == 'nt'):
    MAKE_ALL = ['msbuild','-verbosity:m','/property:Configuration=Release','ALL_BUILD.vcxproj']
    MAKE_INSTALL = ['msbuild','-verbosity:m','/property:Configuration=Release','INSTALL.vcxproj']
else:
    MAKE_ALL = ['make','all','-j',str(CORES)]
    MAKE_INSTALL = ['make','install','-j',str(CORES)]    

VERSION = sys.version_info.major + (sys.version_info.minor / 10)

def subprocess_run(args):
    if(VERSION < 3.5):
        return subprocess.call(args)
    else:
        return subprocess.run(args).returncode


# Argument parsing
argument_parser = argparse.ArgumentParser()
argument_parser.add_argument('-c','--external',action='store_true')
argument_parser.add_argument('-r','--rebuild',action='store_true')

def clean_path(path):
    if(os.path.exists(path)):
        shutil.rmtree(path)

EXTERNAL_BUILD = os.path.join('external','build')

# build targets
def build_external(rebuild):
    result = 0
    result = result + subprocess_run(['git','submodule','update','--init','--recursive'])
    if(rebuild):
        clean_path(EXTERNAL_BUILD) 
    if(not(os.path.exists(EXTERNAL_BUILD))):
        os.mkdir(EXTERNAL_BUILD)
    os.chdir(EXTERNAL_BUILD)
    result = result + subprocess_run(['cmake','..','-Dprotobuf_BUILD_TESTS=OFF'])
    result = result + subprocess_run(MAKE_INSTALL)
    os.chdir('..')
    os.chdir('..')
    return result

def build_library(rebuild):
    result = 0
    if(rebuild):
        clean_path('build')
    if(not(os.path.exists('build'))):
        os.mkdir('build')
    os.chdir('build')
    result = result + subprocess_run(['cmake','..'])
    subprocess_run(MAKE_ALL)
    os.chdir('..')
    return result


def build_main():
    result = 0
    args = argument_parser.parse_args()
    if args.external:
        result = result + build_external(args.rebuild)
    else:
        result = result + build_library(
            args.rebuild)
    return result

if __name__ == '__main__':
    result = 0
    try:
        result = build_main()
    except:
        result = 1
        print(sys.exc_info())
    sys.exit(result)