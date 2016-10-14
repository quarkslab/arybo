import unittest

from arybo.lib import MBA
import arybo.lib.mba_exprs as EX
from arybo.tools import identify 

class IdentifyTest(unittest.TestCase):
    def setUp(self):
        self.mba8 = MBA(8)
        self.x = self.mba8.var('x')
        self.ex = EX.ExprBV(self.mba8.var('x'))

    def check(self, e, ref):
        app = e.vectorial_decomp([self.x])
        eid = identify(app,"x")
        self.assertEqual(EX.eval_expr(eid), EX.eval_expr(ref))

    def test_identify_xor(self):
        e = self.x^0xAA
        self.check(e, EX.ExprXor(self.ex, EX.ExprCst(0xAA, 8)))

    def test_identify_xor(self):
        e = self.x&0xAA
        self.check(e, EX.ExprAnd(self.ex, EX.ExprCst(0xAA, 8)))

    def test_identify_xor_and(self):
        e = (self.x&0x7F)^0xAA
        self.check(e, EX.ExprXor(EX.ExprAnd(self.ex, EX.ExprCst(0x7F, 8)), EX.ExprCst(0xAA,8)))

if __name__ == "__main__":
    unittest.main()
