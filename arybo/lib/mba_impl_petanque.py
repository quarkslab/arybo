# Copyright (c) 2016 Adrien Guinet <adrien@guinet.me>
# Copyright (c) 2016 Ninon Eyrolles <neyrolles@quarkslab.com>
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

import copy
import six
from six.moves import range

from pytanque import symbol, imm, Vector, Matrix, simplify, simplify_inplace, expand_esf_inplace, subs_vectors, subs_exprs, subs_exprs_inplace, analyses, esf_vector, esf, expand_esf, or_to_esf_inplace, Expr

def get_vector_from_cst(nbits, n):
    vec = Vector(nbits)
    vec.set_int_be(n, nbits)
    return vec

def get_int(nbits, v):
    return v.get_int_be()

def popcount(n):
    ret = 0
    while n>0:
        if (n&1) == 1:
            ret += 1
        n >>= 1
    return ret

def next_zero_bit(v):
    v = ~v
    v = (v ^ (v - 1)) >> 1
    return popcount(v)

def evaluate_expr(E, nbits, map_):
    # keys of map_ can be mba variables or symbols
    #   => an mba variable must map to an integer or an mba variable
    #   => a symbol must map to an expression
    keys = []
    values = []
    for k,v in six.iteritems(map_):
        # TOFIX: not a clean test
        if hasattr(k, "vec"):
            keys.extend(k.vec)
            if isinstance(v, six.integer_types):
                values.extend(imm((v>>i)&1) for i in range(k.nbits))
                continue
            if hasattr(v, "vec"):
                v = v.vec
            if isinstance(v, Vector):
                assert(len(v) == len(k.vec))
                values.extend(v)
                continue
            raise ValueError("an MBAVariable must map to an integer value or an MBAVariable!")
        elif isinstance(k, Expr):
            if not k.is_sym():
                raise ValueError("only symbols or MBAVariable can be a key")
            if not isinstance(v, Expr):
                raise ValueError("a symbol can only be mapped to an expression")
            keys.append(k)
            values.append(v)

    E = expand_esf(E)
    simplify_inplace(E)
    subs_exprs_inplace(E, keys, values)
    simplify_inplace(E)
    try:
        return E.get_int_be()
    except RuntimeError:
        return E

def test_N(nbits, X, n):
    ret = imm(1)
    for i in range(nbits):
        if ((n>>i)&1) == 1:
            ret *= X[i]
        else:
            ret *= X[i]+imm(1)
    simplify_inplace(ret)
    return ret


class MBAImpl(object):
    def __init__(self, nbits):
        self.__set_nbits(nbits)
        self.gen_x = Vector(nbits)
        self.use_esf = False
        self.use_opt_mba = True
        for i in range(0, nbits):
            self.gen_x[i] = symbol("__gen_X_%d" % i)

    def __set_nbits(self, nbits):
        self.nbits = nbits
        self.max_uint = (1<<nbits) - 1

    def var_symbols(self, name):
        symbols = [symbol("%s%d" % (name, i)) for i in range(0, self.nbits)]
        M = Vector(self.nbits)
        for i in range(0, self.nbits):
            M[i] = symbols[i]
        return M

    def get_vector_from_cst(self, n):
        return get_vector_from_cst(self.nbits, n)

    def get_int(self, v):
        return get_int(self.nbits, v)

    def identity(self):
        return Matrix.identity(self.nbits)

    def cst_matrix(self, cst):
        return Matrix(self.nbits, self.nbits, lambda i,j: cst)

    def null_matrix(self):
        return Matrix(self.nbits, self.nbits)

#    def iadd_Y(self, X, Y):
#        carry = imm(0)
#        for i in range(0, self.nbits):
#            Xi = X[i]
#            mul_XY = simplify_inplace(Xi*Y[i])
#            Xi += Y[i]
#            simplify_inplace(Xi)
#            carry_new = simplify_inplace(mul_XY + (carry * Xi))
#            Xi += carry
#            simplify_inplace(Xi)
#            carry = carry_new

    def iadd_Y(self, X, Y):
        carry = imm(0)
        ret = Vector(self.nbits)
        if self.use_esf:
            for i in range(0, self.nbits):
                new_carry = esf(2, [X[i], Y[i], carry])
                X[i] += simplify_inplace(Y[i]+carry)
                carry = new_carry
        else:
            for i in range(0, self.nbits):
                sum_XY = simplify_inplace(X[i]+Y[i])
                new_carry = simplify_inplace(X[i]*Y[i] + (carry * sum_XY))
                X[i] = sum_XY + carry
                carry = new_carry
        return ret

    def add_Y(self, X, Y):
        carry = imm(0)
        ret = Vector(self.nbits)
        if self.use_esf:
            for i in range(0, self.nbits):
                ret[i] = simplify_inplace(X[i]+Y[i]+carry)
                carry = esf(2, [X[i], Y[i], carry])
        else:
            for i in range(0, self.nbits):
                sum_XY = simplify_inplace(X[i]+Y[i])
                ret[i] = simplify_inplace(sum_XY+carry)
                carry = simplify_inplace(X[i]*Y[i] + (carry * sum_XY))
        return ret

    def add_n(self, X, n):
        n = n & self.max_uint
        if self.use_esf or not self.use_opt_mba:
            return self.add_Y(X, self.get_vector_from_cst(n))
        else:
            return self.add_n_mba(X, n)

    def add_n_mba(self, X, n):
        null = Vector(self.nbits)
        n = self.get_vector_from_cst(n)
        while (n != null):
            new_X = simplify_inplace(self.xor_Y(X, n))
            n = simplify_inplace(self.and_Y(self.lshift_n(X, 1), self.lshift_n(n, 1)))
            X = new_X
        return (X)

    def sub_n_mba(self, X, n):
        null = Vector(self.nbits)
        n = self.get_vector_from_cst(n)
        while (n != null):
            X = simplify_inplace(self.xor_Y(X, n))
            n = simplify_inplace(self.and_Y(self.lshift_n(X, 1), self.lshift_n(n, 1)))
        return (X)

    def iadd_n(self, X, n):
        n = n & self.max_uint
        if self.use_esf:
            return self.iadd_Y(X, self.get_vector_from_cst(n))
        return self.iadd_n_mba(X, n)

    def iadd_n_mba(self, X, n):
        null = Vector(self.nbits)
        n = self.get_vector_from_cst(n)
        while (n != null):
            carry = simplify_inplace(self.and_Y(X, n))
            self.ixor_Y(X, n)
            simplify_inplace(X)
            n     = self.lshift_n(carry, 1)
        return X

    def iadd_lshifted_Y(self, X, Y, offset):
        if self.use_esf:
            self.iadd_Y(X, self.lshift_n(Y, offset))
            simplify_inplace(X)
            return
        carry = imm(0)
        for i in range(0, self.nbits):
            if i < offset:
                Yi = imm(0)
            else:
                Yi = Y[i-offset]
            Xi = X[i]
            mul_XY = simplify_inplace(Xi*Yi)
            Xi += Yi
            simplify_inplace(Xi)
            carry_new = simplify_inplace(mul_XY + (carry * Xi))
            Xi += carry
            simplify_inplace(Xi)
            carry = carry_new

    def sub_Y(self, X, Y):
        carry = imm(0)
        ret = Vector(self.nbits)
        if self.use_esf:
            for i in range(0, self.nbits):
                ret[i] = simplify_inplace(X[i]+Y[i]+carry)
                carry = esf(2, [X[i]+imm(1), Y[i], carry])
        else:
            for i in range(0, self.nbits):
                sum_XY = simplify_inplace(X[i]+Y[i])
                ret[i] = simplify_inplace(sum_XY+carry)
                carry = simplify_inplace((X[i]+imm(1))*Y[i] + (carry * (sum_XY+imm(1))))
        return ret

    def sub_n(self, X, n):
        n = n & self.max_uint
        return self.sub_Y(X, self.get_vector_from_cst(n))

    def mul_Y(self, X, Y):
        ret = Vector(self.nbits)
        i = 0
        for i in range(0, self.nbits):
            Yi_vec = Vector(self.nbits, Y[i])
            self.iadd_Y(ret, self.lshift_n(X, i) * Yi_vec)
        return ret

    def mul_n_org(self, X, n):
        n = n & self.max_uint
        ret = Vector(self.nbits)
        i = 0
        while n > 0:
            if (n & 1) == 1:
                self.iadd_lshifted_Y(ret, X, i)
            n >>= 1
            i += 1
        return ret

    def mul_n(self, X, n):
        if (n == 1):
            return X
        ret = Vector(self.nbits)
        if (n == 0):
            return ret
        n = n & self.max_uint
        i = 0
        final_sum = 0
        not_x = None
        def compute_not_x(not_x):
            if not_x is None:
                not_x = self.not_X(X)
            return not_x

        while n > 0:
            # Optimisations from the Hacker's delight
            nz = next_zero_bit(n)
            if (nz >= 3):
                not_x = compute_not_x(not_x)
                self.iadd_lshifted_Y(ret, X, nz+i)
                self.iadd_lshifted_Y(ret, not_x, i)
                final_sum += 1<<i
                n >>= nz
                i += nz
            else:
                bits4 = n&0b1111
                if bits4 == 0b1011:
                    not_x = compute_not_x(not_x)
                    self.iadd_lshifted_Y(ret, X, 4+i)
                    self.iadd_lshifted_Y(ret, not_x, 2+i)
                    self.iadd_lshifted_Y(ret, not_x, i)
                    final_sum += 1<<(i+2)
                    final_sum += 1<<i
                    n >>= 4
                    i += 4
                elif bits4 == 0b1101:
                    not_x = compute_not_x(not_x)
                    self.iadd_lshifted_Y(ret, X, 4+i)
                    self.iadd_lshifted_Y(ret, not_x, 1+i)
                    self.iadd_lshifted_Y(ret, not_x, i)
                    final_sum += 1<<(i+1)
                    final_sum += 1<<i
                    n >>= 4
                    i += 4
                else:
                    if (n & 1) == 1:
                        self.iadd_lshifted_Y(ret, X, i)
                    n >>= 1
                    i += 1
        if final_sum > 0:
            self.iadd_n(ret, final_sum & self.max_uint)
        return ret

    def div_n(self, X, n):
        ret = Vector(self.nbits*2+1)
        for i in range(self.nbits):
            ret[i] = X[i]
        nc = (2**self.nbits/n)*n - 1
        for p in range(self.nbits, 2*self.nbits+1):
            if(2**p > nc*(n - 1 - ((2**p - 1) % n))):
                break
        else:
            raise RuntimeError("division: unable to find the shifting count")
        m = (2**p + n - 1 - ((2**p - 1) % n))//n
        self.__set_nbits(2*self.nbits+1)
        ret = self.mul_n(ret, m)
        ret = self.rshift_n(ret, p)
        self.__set_nbits((self.nbits - 1)//2)
        final_ret = Vector(self.nbits)
        for i in range(self.nbits):
            final_ret[i] = ret[i]
        return final_ret

    def phi_X(self, X):
        def f(i, j):
            if i != j:
                return imm(0)
            return X[i]
        return Matrix(self.nbits, self.nbits, f)

    def and_Y(self, X, Y):
        return X*Y
        #return self.phi_X(Y)*X

    def and_n(self, X, n):
        if n < 0:
            n = n & self.max_uint
        ret = Vector(self.nbits)
        for i in range(self.nbits):
            if n & (1<<i):
                ret[i] = X[i]
        return ret
        #return self.phi_X(self.get_vector_from_cst(n))*X

    def and_exp(self, X, e):
        return X*e

    def not_X(self, X):
        return X + self.get_vector_from_cst(self.max_uint)

    def xor_n(self, X, n):
        if n < 0:
            n = n % self
        ret = Vector(self.nbits)
        for i in range(self.nbits):
            if n & (1 << i):
                ret[i] = X[i] + imm(1)
            else:
                ret[i] = X[i]
        return ret

    def xor_exp(self, X, e):
        return X+e

    def xor_Y(self, X, Y):
        return X + Y

    def ixor_Y(self, X, Y):
        X += Y

    def ixor_exp(self, X, e):
        X += e

    def oppose_X(self, X):
        return self.add_n(self.not_X(X), 1)

    def notand_n(self, X, n):
        return self.not_X(self.and_n(X, n))

    def notand_Y(self, X, Y):
        return self.not_X(self.and_Y(X, Y))

    def notand_exp(self, X, e):
        return self.not_exp(self.and_exp(X, e))

    def or_Y(self, X, Y):
        if self.use_esf:
            return esf_vector(2, [X, Y]) + esf_vector(1, [X, Y])
        else:
            return self.xor_Y(self.and_Y(X, Y), self.xor_Y(X, Y))

    def or_exp(self, X, e):
        if self.use_esf:
            E = Vector(self.nbits, e)
            return self.or_Y(X, E)
        else:
            return self.xor_exp(self.and_exp(X, e), self.xor_exp(X, e))

    def or_n(self, X, n):
        ret = Vector(self.nbits)
        for i in range(self.nbits):
            if n & (1<<i):
                ret[i] = imm(1)
            else:
                ret[i] = X[i]
        return ret

    def lshift_n(self, X, n):
        return X>>n

    def rshift_n(self, X, n):
        return X<<n

    def arshift_n(self, X, n):
        n = min(n, self.nbits)
        ret = X<<n
        last_bit = X[self.nbits-1]
        for i in range(self.nbits-n, self.nbits):
            ret[i] = last_bit
        return ret

    def rshift_Y(self, X, Y):
        # Generate 2**Y and multiply X by this
        fds
        pass

    def rol_n(self, X, n):
        # rol(0b(d b c a), 1) = 0b(b c a d)
        # rol(vec(a,b,c,d), 1) = vec(d,a,c,b))
        ret = Vector(self.nbits)
        for i in range(self.nbits):
            ret[i] = X[(i-n)%self.nbits]
        return ret

    def ror_n(self, X, n):
        ret = Vector(self.nbits)
        for i in range(self.nbits):
            ret[i] = X[(i+n)%self.nbits]
        return ret

    def evaluate(self, E, values):
        return evaluate_expr(E, self.nbits, values)

    def vectorial_decomp(self, symbols, X):
        return analyses.vectorial_decomp(symbols, X)


    def permut2expr(self, P, X):
        ret = Vector(self.nbits)
        v0 = P[0]
        nbits_in = (len(P)-1).bit_length()
        for k,v in enumerate(P[1:]):
            v ^= v0
            if v == 0:
                continue
            k += 1
            test = test_N(nbits_in, X, k)
            for i in range(self.nbits):
                if ((v>>i)&1) == 1:
                    ret[i] += test
        for i in range(self.nbits):
            ret[i] += imm((v0 >> i) & 1)
        simplify_inplace(ret)
        return ret

    def symbpermut2expr(self, P, X):
        ret = Vector(self.nbits)
        nbits_in = (len(P)-1).bit_length()
        for k,v in enumerate(P):
            test = test_N(nbits_in, X, k)
            for i in range(self.nbits):
                ret[i] += v[i]*test
        simplify_inplace(ret)
        return ret

    def add_n_matrix(self, n):
        def matrix_v(i, j):
            if i == j:
                return imm(1)
            if i < j:
                return imm(0)
            if i > j:
                mask = (~((1<<(j))-1)) & self.max_uint
                mask2 = ((1<<(i))-1) & self.max_uint
                mask &= mask2
                return imm((n & mask) == mask)

        return Matrix(self.nbits, self.nbits, matrix_v)

    def from_bytes(self, s):
        ret = Vector(self.nbits)
        for i,c in enumerate(six.iterbytes(s)):
            for j in range(8):
                ret[i*8+j] = imm((c>>j)&1)
        return ret

    def to_bytes(self, vec):
        l = (self.nbits+7)//8
        ret = bytearray(l)
        for i,b in enumerate(vec):
            if not b.is_imm():
                raise ValueError("variable does not contain only immediates!")
            b = b.imm_value()
            if b:
                bit_idx = i&7
                byte_idx = i>>3
                ret[byte_idx] |= (b<<bit_idx)
        return bytes(ret)
