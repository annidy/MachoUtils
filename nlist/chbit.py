#!/usr/local/bin/python3
import sys
import os
import mmap


dirname, _ = os.path.split(os.path.abspath(__file__))
args = [os.path.join(dirname, "nlist")]
args += sys.argv[1:-1]
proc_file = args[-1]
proc_symbol = sys.argv[-1]
print(args)

import subprocess
result = subprocess.run(args, stdout=subprocess.PIPE)
for line in result.stdout.decode('utf-8').splitlines():
    try:
        addr, name = line.split()
        if name == proc_symbol:
            off = int(addr, 16)+4
            print("write "+hex(off), end="")


            with open(proc_file, 'rb', 0) as fd:
                fd.seek(off)
                oldb = fd.read(1)
            
            nold = int.from_bytes(oldb, byteorder='little')
            if nold & 1 == 0: 
                nnew = nold + 1
            else:
                nnew = nold - 1
 
            with open(proc_file, 'r+b', 0) as fd:
                mm = mmap.mmap(fd.fileno(), 0)
                mm.seek(off)
                mm.write_byte(nnew)
                mm.close()

            print(", "+hex(nold)+" -> "+hex(nnew))

    except:
        pass
