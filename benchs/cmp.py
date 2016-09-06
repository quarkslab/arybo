#!/usr/bin/env python3
#

import sys

if len(sys.argv) <= 2:
    print("Usage: %s ref new" % sys.argv[0])
    sys.exit(1)

reff = sys.argv[1]
newf = sys.argv[2]


class BenchRes:
    def __init__(self, name, time_ms, mem_mb):
        self.name = name
        self.time_ms = time_ms
        self.mem_mb = mem_mb

    def __repr__(self):
        return "%s\t%0.2f\t%0.2f" % (self.name, self.time_ms, self.mem_mb)

    def str_res(self):
        return "%0.2f\t%0.2f" % (self.time_ms, self.mem_mb)

def read_benchs(f):
    fd = open(f, "r")
    ret = list()
    for l in fd:
        l = l.strip().split('\t')
        br = BenchRes(l[0], float(l[1]), float(l[2]))
        ret.append(br)
    ret = sorted(ret, key=lambda r: r.name)
    return ret

def gain(old, new):
    return old/new

def gain_time(old, new):
    return gain(old.time_ms, new.time_ms)

def gain_mem(old, new):
    return gain(old.mem_mb, new.mem_mb)

ref = read_benchs(reff)
new = read_benchs(newf)

#print(ref)
#print(new)

print("name\ttime_old\ttime_new\ttime_gain\tmem_old\tmem_new\tmem_reduction")
iref = 0
inew = 0
while iref < len(ref) and inew < len(new):
    br_o = ref[iref]
    br_n = new[inew]
    if br_o.name == br_n.name:
        print("%s\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f" % (br_o.name, br_o.time_ms, br_n.time_ms, gain_time(br_o, br_n), br_o.mem_mb, br_n.mem_mb, gain_mem(br_o, br_n)))
        iref += 1
        inew += 1
    elif br_o.name < br_n.name:
        print("%s\t%0.2f\tNA\tNA\t%0.2f\tNA\tNA" % (br_o.name, br_o.time_ms, br_o.mem_mb))
        iref += 1
    else:
        print("%s\tNA\t%0.2f\tNA\tNA\t%0.2f\tNA" % (br_n.name, br_n.time_ms, br_n.mem_mb))
        inew += 1
