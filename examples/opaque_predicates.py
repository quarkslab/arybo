from arybo.lib import MBA


mba32 = MBA(32)
X = mba32.var('X')
Y = mba32.var('Y')
X.always_simplify()
Y.always_simplify()

op = (((X | 0xFDFFDBF3 ) - ((X | 0x7C5F9972) & 0x0248AD2E)) + 0x248AD36)
print(hex(op.to_cst()))

op = ((~X | 0x7AFAFA69) & 0xA061440) + (X & 0x1050504 | 0x101890D)
print(hex(op.to_cst()))

op = ((~X | 0x3BBDA8B5) & 0xB21C528) + (X & 0x4040474A | 0x40C00A46)
print(hex(op.to_cst()))

op = (X & 0xD281D094) - 1461583213 - (X | 0x2D7E2F6B)
print(hex(op.to_cst()))
