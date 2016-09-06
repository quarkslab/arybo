from arybo.lib import MBA
import sys

mba = MBA(int(sys.argv[1]))
X = mba.var('X')
Y = mba.var('Y')

xor = X+Y - ((X&Y) << 1)
print(xor)
