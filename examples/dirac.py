from arybo.lib import MBA, boolean_expr_solve
mba = MBA(64)
x = mba.var('x')
def f(X):
  T = ((X+1)&(~X))
  C = ((T | 0x7AFAFA697AFAFA69) & 0x80A061440A061440)\
      + ((~T & 0x10401050504) | 0x1010104)
  return C
r = f(x)
sols = boolean_expr_solve(r[63], x, 1)
C0 = sols[0].get_int_be()
print(hex(C0))
print(hex(f(0)))
print(hex(f(C0)))
