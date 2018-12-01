#!/usr/local/bin/python3
import sys
import os
import mmap
print(sys.argv)

dirname, filename = os.path.split(os.path.abspath(__file__))
nlist = os.path.join(dirname, "nlist")

import subprocess
result = subprocess.run([nlist, sys.argv[1]], stdout=subprocess.PIPE)
for line in result.stdout.decode('utf-8').splitlines():
    addr, name = line.split()
    if name == sys.argv[2]:
        off = int(addr, 16)+4
        print("write "+hex(off), end="")

        with open(sys.argv[1], 'rb', 0) as fd:
            fd.seek(off)
            print(", old "+str(fd.read(1)))

        with open(sys.argv[1], 'r+b', 0) as fd:
            mm = mmap.mmap(fd.fileno(), 0)
            mm.seek(off)
            mm.write_byte(0x0f)
            mm.close()