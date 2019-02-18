#!/usr/local/bin/python3
# -*- coding: utf-8 -*-

'''
dupsymbols a1
内部可能有符号冲突，关闭Strip Dead Code选项后会出现编译器错误
'''
import sys
import os
from pathlib import Path
import shutil
import subprocess as sp
import re

a1 = sys.argv[1]

class file(object):

    def __init__(self, a):
        self.symbol = set()
        self.name = a

    def add(self, sym):
        self.symbol.add(sym)

    def test(self, file):
        comm = (file.symbol & self.symbol)
        if len(comm) > 0:
            print(file.name, "<->", self.name)
            for n in comm:
                print(n)
    pass


def ar_symbol(path):
    proc = sp.Popen(['nm', '-g', '-arch', 'arm64', path], stdout=sp.PIPE,
                       stderr=sp.DEVNULL, universal_newlines=True)

    files = []

    for line in proc.stdout:
        m = re.search('(?<=\\().+(?=\\))', line)
        if m:
            afile = file(m.group())
            files.append(afile)
            continue
        
        m = re.search('(?<=T ).*', line)
        if m:
            if afile != None:
                afile.add(m.group())

    return files

def run(path):
    files = ar_symbol(path)
    print(files)
    for i in range(len(files)-1):
        for j in range(i+1, len(files)):
            files[i].test(files[j])

run(a1)
