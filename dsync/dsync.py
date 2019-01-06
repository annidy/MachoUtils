#!/usr/local/bin/python3
'''
dsync 目录1 目录2
删除<目录2>中在<目录1>存在的文件
'''
import sys
import os
from pathlib import Path
import shutil

RED="\u001b[31m"
GREEN="\u001b[32m"

src_dir = sys.argv[1]
src_p = Path(src_dir)
names = []
for i in src_p.glob('*.o'):
    names.append(i.name)

dst_dir = sys.argv[2]
dst_p = Path(dst_dir)
for i in dst_p.iterdir():
    if names.count(i.name):
        os.remove(i.absolute())
        print(GREEN+"delete ", i.name)
        names.remove(i.name)

for n in names:
    print(RED+"miss ", n)