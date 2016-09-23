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

from sympy import *
import copy
from six.moves import range

def mexpand(M):
    T = copy.deepcopy(M)
    for i in range(len(T)):
        T[i] = expand(T[i])
    return T

def simp_matrix(M, symbols):
    for i in range(0,len(M)):
        M[i] = simp_poly(M[i], symbols)
    return M

def fast_expr_mod2(e):
    if isinstance(e, Symbol):
        return e
    if isinstance(e, Integer):
        return e%2
    if isinstance(e, Pow):
        return Pow(fast_expr_mod2(e.args[0]), e.args[1])
    args = [fast_expr_mod2(a) for a in e.args]
    return type(e)(*args)

def fast_matrix_inplace_mod2(M):
    for i in range(0, len(M)):
        M[i] = fast_expr_mod2(M[i])
    return M

def simp_poly(p, symbols):
    p = fast_expr_mod2(p)
    p = expand(p, deep=True, modulus=2, mul=True, multinomial=True, power_exp=False, power_base=False, log=False, complex=False, func=False, trig=False)
    p = simp_expr(p, Pow, lambda p: p.args[0])
    p = fast_expr_mod2(p)
    return p

def simp_expr(e, type_, f):
    if isinstance(e, Symbol):
        return e
    if isinstance(e, Integer):
        return e
    if isinstance(e, type_):
        return f(e)
    args = []
    for a in e.args:
        if isinstance(a, type_):
            args.append(f(a))
        else:
            args.append(simp_expr(a, type_, f))
    return type(e)(*args)

def mod2(M):
    T = copy.deepcopy(M)
    for i in range(len(T)):
        T[i] = T[i] % 2
    return T

def test_matrix(E, value):
    v = get_vector(value)
    return simp_matrix(E.subs([(e,v[0]), (f,v[1]), (g,v[2]), (h,v[3])]))

def get_vector(nbits, n):
    n = n%(1<<nbits)
    l = [int(digit) for digit in bin(n)[2:][::-1]]
    while len(l) < nbits:
        l.append(0)
    return Matrix(l)

def get_int(nbits, v):
    res = 0
    for i in range(0, nbits):
        res += int(v[i])<<i
    return res

def evaluate_expr_base(E, nbits, symbols, bits_values):
    return get_int(nbits, mod2(E.subs(([(symbols[i],bits_values[i]) for i in range(0, len(symbols))]))))

def evaluate_expr(E, nbits, values):
    # values = {MBAVariable1: int, MBAVariable2: int}
    symbols = list()
    bits_values = list()
    for mbav, v in values.items():
        symbols.extend(mbav.symbols)
        bits_values.extend(get_vector(nbits, v))
    return evaluate_expr_base(E, nbits, symbols, bits_values)

class MBAImpl(object):
    def __init__(self, nbits):
        self.nbits = nbits
        self.max_uint = (1<<nbits) - 1

    def get_vector(self, n):
        return get_vector(self.nbits, n)

    def get_int(self, v):
        return get_int(self.nbits, n)

    def identity(self):
        def f(i,j):
            if i == j:
                return 1
            else:
                return 0
        return Matrix(self.nbits, self.nbits, f)

    def cst_matrix(self, cst):
        return Matrix(self.nbits, self.nbits, lambda i,j: cst)

    def null_matrix(self):
        return self.cst_matrix(0)

    def add_Y(self, X, Y):
        carry = 0
        ret = [0]*self.nbits
        for i in range(0, self.nbits):
            ret[i] = fast_expr_mod2(X[i]+Y[i]+carry)
            carry = fast_expr_mod2(X[i]*Y[i] + (carry * (X[i]+Y[i])))
        return Matrix(ret)

    def add_n(self, X, n):
        return self.add_Y(X, self.get_vector(n))

    def mul_n(self, X, n):
        # Technique de roumain powered
        ret = self.null_matrix()
        i = 0
        while n > 0:
            if (n & 1) == 1:
                ret = self.add_Y(ret, self.lshift_n(X, i))
            n >>= 1
            i += 1
        return ret

    def phi_X(self, X):
        m = []
        for i in range(0, self.nbits):
            v = [0]*self.nbits
            v[i] = X[i]
            m.append(v)
        return Matrix(m)

    def and_Y(self, X, Y):
        return fast_matrix_inplace_mod2(self.phi_X(Y)*X)

    def and_n(self, X, n):
        return fast_matrix_inplace_mod2(self.phi_X(self.get_vector(n))*X)

    def not_X(self, X):
        return fast_matrix_inplace_mod2(X + self.get_vector(self.max_uint))

    def xor_n(self, X, n):
        return fast_matrix_inplace_mod2(X + self.get_vector(n))

    def oppose_X(self, X):
        return fast_matrix_inplace_mod2(self.add_n(self.not_X(X), 1))

    def xor_Y(self, X, Y):
        return fast_matrix_inplace_mod2(X + Y)

    def notand_n(self, X, n):
        return fast_matrix_inplace_mod2(self.not_X(self.and_n(X, n)))

    def notand_Y(self, X, Y):
        return fast_matrix_inplace_mod2(self.not_X(self.and_Y(X, Y)))

    def or_Y(self, X, Y):
        return fast_matrix_inplace_mod2(self.notand_Y(self.notand_Y(X, X), self.notand_Y(Y, Y)))

    def or_n(self, X, n):
        return fast_matrix_inplace_mod2(self.or_Y(X, self.get_vector(n)))

    def lshift_n(self, X, n):
        l = [0]*n + X[0:self.nbits-n]
        return Matrix(l)

    def rshift_n(self, X, n):
        l = X[(-(self.nbits-n)):] + [0]*n
        return Matrix(l)

    def evaluate(self, E, values):
        return evaluate_expr(E, self.nbits, values)
    eval = evaluate
