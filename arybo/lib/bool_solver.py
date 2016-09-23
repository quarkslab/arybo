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

import itertools
from six.moves import range

from pytanque import imm, Vector, imm

# TODO: add the relevant interface into SymbolsSet and use it here!
class BooleanSystemSolver:
    def __init__(self):
        self.I1 = set()
        self.I0s = []

    def __getSyms(self, e):
        if e.is_mul():
            syms = e.args()
            for s in syms:
                if not s.is_sym():
                    raise ValueError("e must be a multiplication of symbols or a symbol")
        elif e.is_sym():
            syms = (e,)
        else:
            raise ValueError("e must be a multiplication of symbols or a symbol")
        return map(lambda s: s.sym_idx(), syms)

    def equalsOne(self, e):
        syms = self.__getSyms(e)
        self.I1.update(syms)

    def equalsZero(self, e):
        syms_set = set(self.__getSyms(e))
        self.I0s.append(syms_set)

    def __idx(self, idx, base, nbits):
        idx -= base
        assert(idx >= 0 and idx < nbits)
        return idx

    def solve(self, X):
        nbits = len(X.vec)
        idx_base = X[0].sym_idx()
        for I0 in self.I0s:
            I0.difference_update(self.I1)
            if (len(I0) == 0):
                return []
        self.I0s = [I0 for I0 in self.I0s if len(I0) > 0]

        fix1 = Vector(X.vec)
        for idx in self.I1:
            idx = self.__idx(idx, idx_base, nbits)
            fix1[idx] = imm(1)
        if (len(self.I0s) == 0):
            return [fix1]

        ret = []
        def iter_zeros(idxes, I0s):
            if len(I0s) == 0:
                yield idxes
                return

            for i,I0 in enumerate(I0s):
                if I0.isdisjoint(idxes):
                    next_I0 = I0
                    next_idx = i
                    break
            else:
                yield idxes
                return

            for idx in next_I0:
                new_idxes = idxes + [idx]
                for z in iter_zeros(new_idxes, I0s[next_idx+1:]):
                    yield z

        for idxes in iter_zeros([], self.I0s):
            new = Vector(fix1)
            for idx in idxes:
                new[self.__idx(idx, idx_base, nbits)] = imm(0)
            ret.append(new)
        return ret

def boolean_expr_solve(e, X, v=1):
    if e.is_add():
        monomes = e.args()
    elif e.is_sym():
        monomes = (e,)
    else:
        raise ValueError("e should be normalized!")

    len_monomes = len(monomes)
    if len_monomes == 0:
        return []
    last = monomes[len_monomes-1]
    if last.is_imm() and last.imm_value() == True:
        v = int(not v)
        len_monomes -= 1

    if v not in (0,1):
        raise ValueError("v should be equal to 0 or 1!")

    ret = []
    all_monomes = set(range(len_monomes))
    def solve(ones):
        BSS = BooleanSystemSolver()
        for m in ones:
            BSS.equalsOne(monomes[m])
        for m in all_monomes.difference(ones):
            BSS.equalsZero(monomes[m])
        ret.extend(BSS.solve(X))

    # If v equals to 1, we want an odd number of ones.
    # On the contrary, we need an even number of ones (zero included).
    for nones in range(v, len_monomes+1, 2):
        for c in itertools.combinations(range(len_monomes), nones):
            solve(c)
    return ret
