#!/usr/bin/env python3
#

import sys
import subprocess

def do_bench(cmd):
    subp = subprocess.Popen("/usr/bin/time -f '%e %M' " + cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    p = subp.communicate()
    rc = subp.returncode
    if rc != 0:
        print("Error while running '%s' (ret %d): %s" % (cmd, rc, p[1]), file=sys.stderr)
        return None
    time = p[1]
    ret = time.decode('ascii').strip().split(' ')
    return ret

benchs = open("benchs", "r")
for b in benchs:
    tmp = b.strip().split(' ')
    name = tmp[0]
    cmd = ' '.join(tmp[1:])
    ret = do_bench(cmd)
    if ret != None:
        time_s, mem_kb = ret
        print("%s\t%0.2f\t%0.2f" % (name, float(time_s)*1000.0, float(mem_kb)/1024.0))
