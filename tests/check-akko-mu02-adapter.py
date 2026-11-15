#!/usr/bin/env python3
"""Run the fake-HID tests against real OpenRGB framework objects after a full build."""
from pathlib import Path
import re
import shlex
import subprocess

build = Path(__file__).resolve().parents[1] / 'build'
text = (build / 'Makefile').read_text().replace('\\\n', ' ')
variables = dict(re.findall(r'^(\w+)\s*=\s*(.*)$', text, re.M))


def flags(name):
    value = variables.get(name, '')
    for _ in range(10):
        new = re.sub(r'\$\((\w+)\)', lambda m: variables.get(m[1], ''), value)
        if new == value:
            return shlex.split(value)
        value = new
    raise ValueError('Unresolved qmake flags: ' + name)


def run(args):
    subprocess.run(args, cwd=build, check=True)


run(['g++', '-std=c++17', '-fPIC', '-pthread', '-DAKKO_MU02_TEST_ADAPTER',
     *flags('INCPATH'), '-c', '../tests/akko-mu02.cpp', '-o', 'mu02-adapter-test.o'])
run(['objcopy', '--redefine-sym', 'main=OpenRGB_Application_Main',
     'main_FreeBSD_Linux_MacOS.o', 'mu02-main-renamed.o'])
objects = ['mu02-main-renamed.o' if o == 'main_FreeBSD_Linux_MacOS.o' else o
           for o in flags('OBJECTS')]
run(['g++', '-o', 'mu02-adapter-test', 'mu02-adapter-test.o', *objects,
     *flags('LFLAGS'), *flags('LIBS')])
run([str(build / 'mu02-adapter-test')])
