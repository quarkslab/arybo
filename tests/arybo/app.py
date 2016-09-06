from arybo.lib import MBA, simplify
from arybo.tools import is_app_inversible, app_inverse

import unittest

class AppTest(unittest.TestCase):
    def setUp(self):
        mba = MBA(4)
        mba.use_esf = False
        self.X = mba.var('X')

    def test_app1(self):
        X = self.X
        A = (7*X+5).vectorial_decomp([X])
        Ainv = app_inverse(A)

        self.assertIsNotNone(Ainv)
        ident = simplify(Ainv(A(X.vec)))
        self.assertEqual(ident, self.X.vec)

    def test_app2(self):
        X = self.X
        A = (16+11*X+12*X*X+2*X*X*X).vectorial_decomp([X])
        Ainv = app_inverse(A)

        self.assertIsNotNone(Ainv)
        ident = simplify(Ainv(A(X.vec)))
        self.assertEqual(ident, self.X.vec)
