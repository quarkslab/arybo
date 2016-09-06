from arybo.lib import MBA
import unittest

class TestMBA(unittest.TestCase):
    def setUp(self):
        mba6 = MBA(6)
        mba6.use_esf = False
        mba8 = MBA(8)
        mba8.use_esf = False

        self.X6 = mba6.var('X')
        self.Y6 = mba6.var('Y')
        self.X8 = mba8.var('X')

    def test_xor_5C(self):
        X = self.X8
        v0 = X
        v0.always_simplify()
        v0 = v0*0xe5 + 0xF7
        v3 = (((v0*0x26)+0x55)&0xFE)+(v0*0xED)+0xD6
        v4 = ((((((- (v3*0x2))+0xFF)&0xFE)+v3)*0x03)+0x4D)
        v5 = (((((v4*0x56)+0x24)&0x46)*0x4B)+(v4*0xE7)+0x76)
        v7 = ((((v5*0x3A)+0xAF)&0xF4)+(v5*0x63)+0x2E)
        v6 = (v7&0x94)
        v8 = ((((v6<<1)+(-v7))*0x67)+0xD)
        result = ((v8*0x2D)+(((v8*0xAE)|0x22)*0xE5)+0xC2)
        result = (result-0xF7)*0xed

        ref = X^0x5C
        self.assertEqual(ref, result)

    def test_gen_mba1(self):
        X = self.X6
        Y = self.Y6
        xor = X+Y - ((X&Y) << 1)
        xor = xor.simplify()

        self.assertEqual(xor, (X^Y))

    def test_gen_mba2(self):
        X = self.X6
        Y = self.Y6
        xor = (X | Y) - (X & Y)
        xor = xor.simplify()

        self.assertEqual(xor, (X^Y))

    def test_gen_mba3(self):
        X = self.X6
        Y = self.Y6
        xor = (X & ~(Y)) - (X & Y)  + Y
        xor = xor.simplify()

        self.assertEqual(xor, (X^Y))
