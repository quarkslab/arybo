import unittest
import random
import operator

from arybo.lib import MBA, simplify_inplace, expand_esf_inplace
from arybo.tools import triton2arybo, triton_available

if triton_available:
    import triton as TT
    import triton.ast as TAst

@unittest.skipIf(triton_available == False, "skipping Triton-related tests as it is not available")
class TritonTest(unittest.TestCase):
    def setUp(self):
        # Initialize the engine
        TT.setArchitecture(TT.ARCH.X86)

        self.mba8  = MBA(8)
        self.mba16 = MBA(16)
        self.mba8.use_esf  = True
        self.mba16.use_esf = True
        # Useful variables using duringtests
        self.x8_t  = TAst.variable(TT.newSymbolicVariable(8))
        self.y8_t  = TAst.variable(TT.newSymbolicVariable(8))
        self.x16_t = TAst.variable(TT.newSymbolicVariable(16))
        self.y16_t = TAst.variable(TT.newSymbolicVariable(16))
        self.x8  = self.mba8.var(self.x8_t.getValue())
        self.y8  = self.mba8.var(self.y8_t.getValue())
        self.x16 = self.mba16.var(self.x16_t.getValue())
        self.y16 = self.mba16.var(self.y16_t.getValue())

    # Helpers
    def astEquals(self, ast, v):
        self.assertEqual(triton2arybo(ast,use_esf=True), v)

    # Tests
    def test_leaves(self):
        c  = random.choice(range(0xff))
        self.astEquals(
            TAst.bv(c, 8),
            self.mba8.from_cst(c))

        self.astEquals(
            self.x8_t,
            self.x8)

    def test_zx_sx(self):
        self.astEquals(
            TAst.zx(16, TAst.bv(0xff, 8)),
            self.mba16.from_cst(0x00ff))

        self.astEquals(
            TAst.sx(16, TAst.bv(0xff, 8)),
            self.mba16.from_cst(0xffff))

    def test_extract_contract(self):
        self.astEquals(
            TAst.extract(7, 0, TAst.bv(0xffff, 16)),
            self.mba8.from_cst(0xff))

        self.astEquals(
            TAst.concat([TAst.bv(0xff, 8), TAst.bv(0x00, 8)]),
            self.mba16.from_cst(0xff00))

    def test_unaryops(self):
        self.astEquals(
            TAst.bvnot(self.x8_t),
            ~self.x8)

        self.astEquals(
            TAst.bvneg(self.x8_t),
            -self.x8)

    def test_binaryops(self):
        ops = (
            (TAst.bvadd,  operator.add),
            (TAst.bvsub,  operator.sub),
            (TAst.bvand,  operator.and_),
            (TAst.bvor,   operator.or_),
            (TAst.bvxor,  operator.xor),
            (TAst.bvmul,  operator.mul),
            (TAst.bvnand, lambda x,y: ~(x&y)),
            (TAst.bvnor,  lambda x,y: ~(x|y)),
            (TAst.bvxnor, lambda x,y: ~(x^y))
        )
        for op in ops:
            self.astEquals(
                op[0](self.x8_t,self.y8_t),
                op[1](self.x8,self.y8))

        # One udiv test because it can take a lot of time...
        e = TAst.bvudiv(self.x8_t, TAst.bv(15, 8))
        self.astEquals(e, self.x8.udiv(15))

    def test_shifts(self):
        # Triton interface is not consistant between sh{r,l} and ro{l,r}.
        # For the first kind, it can take any AstNode. For the second one, an
        # integer is forced.
        ops = (
            (TAst.bvlshr, operator.rshift),
            (TAst.bvshl,  operator.lshift),
        )
        for op in ops:
            for s in range(9):
                self.astEquals(
                    op[0](TAst.bv(s, 8), self.x8_t),
                    op[1](self.x8, s))

        ops = (
            (TAst.bvrol,  lambda x,n: x.rol(n)),
            (TAst.bvror,  lambda x,n: x.ror(n))
        )
        for op in ops:
            for s in range(9):
                self.astEquals(
                    op[0](s, self.x8_t),
                    op[1](self.x8, s))

    def test_mba(self):
        # x^y = (x+y) - ((x&y)<<1)
        e = TAst.bvsub(
                TAst.bvadd(self.x8_t, self.y8_t),
                TAst.bvshl(TAst.bv(1, 8),
                    TAst.bvand(self.x8_t, self.y8_t)))
        ea = triton2arybo(e,use_esf=True)
        simplify_inplace(expand_esf_inplace(ea))
        self.assertEqual(ea, self.x8^self.y8)

if __name__ == "__main__":
    unittest.main()
