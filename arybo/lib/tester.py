# Copyright (c) 2016 Adrien Guinet <adrien@guinet.me>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

from six.moves import range

class MBATester:
    def __init__(self, mba):
        self.mba = mba
        self.modulus = 1<<self.nbits
        self.X = self.mba.var('X')
        self.Y = self.mba.var('Y')

    @property
    def nbits(self):
        return self.mba.nbits

    def __test_op(self, P, op):
        ret = True
        for i in range(0, 1<<self.nbits):
            for j in range(0, 1<<self.nbits):
                ref = op(i, j)
                v = self.mba.evaluate(P, {self.X: i, self.Y: j})
                if ref != v:
                    print("error (i/j/ref/value)", i, j, ref, v)
                    ret = False
        return ret

    def test_xor(self):
        print("test xor")
        P = self.mba.xor_Y(self.X.vec, self.Y.vec)
        op_xor = lambda a,b: a^b
        return self.__test_op(P, op_xor)

    def test_and(self):
        print("test and")
        P = self.mba.and_Y(self.X.vec, self.Y.vec)
        op_and = lambda a,b: a&b
        return self.__test_op(P, op_and)

    def test_mulgen(self):
        print("test mulgen")
        P = self.mba.mul_Y(self.X.vec, self.Y.vec)
        op_mul = lambda a,b: (a*b)%(self.modulus)
        return self.__test_op(P, op_mul)

    def test_notand(self):
        print("test notand")
        P = self.mba.notand_Y(self.X.vec, self.Y.vec)
        op_notand = lambda a,b: (~(a&b))%(self.modulus)
        return self.__test_op(P, op_notand)

    def test_or(self):
        print("test or")
        P = self.mba.or_Y(self.X.vec, self.Y.vec)
        op_notand = lambda a,b: (a|b)%(self.modulus)
        return self.__test_op(P, op_notand)

    def test_add(self):
        print("test add")
        P = self.mba.add_Y(self.X.vec, self.Y.vec)
        op_add = lambda a,b: (a+b)%(self.modulus)
        return self.__test_op(P, op_add)

    def test_sub(self):
        print("test sub")
        P = self.mba.sub_Y(self.X.vec, self.Y.vec)
        op_sub = lambda a,b: (a-b)%(self.modulus)
        return self.__test_op(P, op_sub)

    def test_iadd(self):
        print("test iadd")
        P = self.mba.var('X')
        self.mba.iadd_Y(P.vec, self.Y.vec)
        op_add = lambda a,b: (a+b)%(self.modulus)
        return self.__test_op(P.vec, op_add)

    def test_mul(self):
        print("test mul")
        ret = True
        op_mul = lambda a,b: (a*b)%(self.modulus)
        for j in range(1<<self.nbits):
            P = self.mba.mul_n(self.X.vec, j)
            for i in range(1<<self.nbits):
                v = self.mba.evaluate(P, {self.X: i})
                ref = op_mul(i, j)
                if v != ref:
                    print("error (i/j/ref/value)", i, j, ref, v)
                    ret = False
        return ret

    def test_div(self):
        print("test div")
        ret = True
        op_div = lambda a,b: (a//b)%(self.modulus)
        for j in range(1, 1<<self.nbits):
            P = self.mba.div_n(self.X.vec, j)
            for i in range(0, 1<<self.nbits):
                v = self.mba.evaluate(P, {self.X: i})
                ref = op_div(i, j)
                if v != ref:
                    print("error", i, j, ref, v)
                    ret = False
        return ret

#    def test_imul(self):
#        op_mul = lambda a,b: (a*b)%(self.modulus)
#        for j in range(0, 1<<self.nbits):
#            P = self.mba.var('X')
#            self.mba.imul_n(P.vec, j)
#            print(j, P.vec)
#            for i in range(0, 1<<self.nbits):
#                v = self.mba.evaluate(P.vec, {self.X: i})
#                ref = op_mul(i, j)
#                if v != ref:
#                    print("error", i, j, ref, v)

    def test_oppose(self):
        print("test oppose")
        ret = True
        P = self.mba.oppose_X(self.X.vec)
        for i in range(0, self.modulus):
            ref = (-i)%self.modulus
            v = self.mba.evaluate(P, {self.X: i})
            if ref != v:
                print("error oppose", i, ref, v)
                ret = False
        return ret

    def test_shifts(self):
        print("test shifts")
        ret = True
        P = self.mba.lshift_n(self.X.vec, 1)
        if self.mba.evaluate(P, {self.X: 4}) != 8:
            print("error lshift")
            ret = False

        P = self.mba.rshift_n(self.X.vec, 2)
        if self.mba.evaluate(P, {self.X: 8}) != 2:
            print("error rshift")
            ret = False
        return ret

    def test_all(self):
        ret = True
        ret &= self.test_xor()
        ret &= self.test_and()
        ret &= self.test_or()
        ret &= self.test_notand()
        ret &= self.test_add()
        ret &= self.test_sub()
        ret &= self.test_iadd()
        ret &= self.test_mul()
        ret &= self.test_div()
        ret &= self.test_mulgen()
        ret &= self.test_oppose()
        ret &= self.test_shifts()
        return ret
