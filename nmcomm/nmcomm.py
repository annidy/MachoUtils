#!/usr/local/bin/python3
'''
nmcomm a1 a2
对比相同导出符号
'''
import sys
import os
from pathlib import Path
import shutil
import subprocess as sp

a1 = sys.argv[1]
a2 = sys.argv[2]
def run(path):
    proc = sp.Popen(['nm', '-gjU', '-arch', 'arm64', path], stdout=sp.PIPE,
                       stderr=sp.DEVNULL, universal_newlines=True)
    symbols = [i.strip() for i in proc.stdout if i.startswith('_')]
    return symbols


s1 = set(run(a1))
s2 = set(run(a2))

for n in (s1 & s2):
    print(n)