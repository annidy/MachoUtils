#!/usr/local/bin/python3
# -*- coding: utf-8 -*-

'''
nmcomm a1 a2
对比相同导出符号
'''
import sys
import os
from pathlib import Path
import shutil
import subprocess as sp
import yaml

a1 = sys.argv[1]
a2 = sys.argv[2]

def tbd_symbol(path):
    symbols = []
    event_iter = iter(yaml.parse(open('/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS.sdk/usr/lib/libSystem.tbd', 'r')))
    for event in event_iter:
        if isinstance(event, yaml.ScalarEvent) and event.value == "symbols":
            for symbol in event_iter:
                if isinstance(symbol, yaml.ScalarEvent):
                    symbols.append(symbol.value)
                if isinstance(symbol, yaml.SequenceEndEvent):
                    break
    return symbols

def ar_symbol(path):
    proc = sp.Popen(['nm', '-gjU', '-arch', 'arm64', path], stdout=sp.PIPE,
                       stderr=sp.DEVNULL, universal_newlines=True)
    symbols = [i.strip() for i in proc.stdout if i.startswith('_')]
    return symbols

def run(path):
    if path.endswith(".tbd"):
        return tbd_symbol(path)
    else:
        return ar_symbol(path)

s1 = set(run(a1))
s2 = set(run(a2))

for n in (s1 & s2):
    print(n)
