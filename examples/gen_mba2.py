from arybo.lib import MBA
import sys

m = MBA(int(sys.argv[1]))
X = m.var('X')
Y = m.var('Y')

res = (X+Y) - (X|Y)
print(res)
