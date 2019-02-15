#!/usr/local/bin/python3
# -*- coding: utf-8 -*-
import sys
import os
import mmap

'''
Set/Unset
chbit.py .a symbol
'''

dirname, _ = os.path.split(os.path.abspath(__file__))
args = [os.path.join(dirname, "nlist"), "-arch", "all"]
args += sys.argv[1:-1]
proc_file = args[-1]
proc_symbol = sys.argv[-1]

import subprocess
result = subprocess.run(args, stdout=subprocess.PIPE)
for line in result.stdout.decode('utf-8').splitlines():
    try:
        addr, name = line.split()
        if name == proc_symbol:
            off = int(addr, 16)+4

            with open(proc_file, 'rb', 0) as fd:
                fd.seek(off)
                oldb = fd.read(1)
            
            nold = int.from_bytes(oldb, byteorder='little')
            if nold & 1 == 0: 
                nnew = nold + 1
                tag = "Set"
            else:
                nnew = nold - 1
                tag = "Unset"
 
            with open(proc_file, 'r+b', 0) as fd:
                mm = mmap.mmap(fd.fileno(), 0)
                mm.seek(off)
                mm.write_byte(nnew)
                mm.close()
            
            print(hex(off)+"\t"+tag)

    except:
        pass
