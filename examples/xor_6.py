from arybo.lib import MBA
mba = MBA(4)

p = mba.var('X')
v0 = (p+2)&15;
bp = (v0>>1)&1;
cp = (v0>>2)&1;
b = (p>>1)&1;
v1 = v0 ^ ((bp << 2) | ((b & (bp ^ cp)) << 3));
v2 = v1 ^ (b << 3);
VD = v2.vectorial_decomp([p])

print(v2)
print("====")
print("Cst = " + hex(VD.cst().get_int_be()))
