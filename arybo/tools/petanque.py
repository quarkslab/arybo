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

import six
from six.moves import range

import networkx as nx
import collections, itertools

from pytanque import Vector, subs_exprs, SymbolsSet, SymbolsHist, esf
from arybo.lib import MBA, MBAVariable, expand_esf, simplify, simplify_inplace, expand_esf_inplace

try:
    from scipy import special
    binom = special.binom
except ImportError:
    # From https://en.wikipedia.org/wiki/Binomial_coefficient
    def binom(n,k):
        if k < 0 or k > n:
            return 0
        if k == 0 or k == n:
            return 1
        k = min(k, n - k) # take advantage of symmetry
        c = 1
        for i in range(k):
            c = c * (n - i) / (i + 1)
        return c


def get_depends(e):
    if e.is_add():
        for a in e.args():
            for d in get_depends(a): yield d
    if e.is_mul():
        for a in e.args():
            if a.is_sym():
                yield a

def get_depends_as_set(e):
    ret = SymbolsSet()
    def set_depends_rec(e, set_):
        if e.is_sym():
            set_.insert(e)
            return
        if e.has_args():
            for a in e.args():
                set_depends_rec(a, set_)
    set_depends_rec(e, ret)
    return ret

def is_app_inversible(A):
    M = A.matrix()
    NL = A.nl()
    V = A.cst()
    Minv = M.inverse()
    if Minv.ncols() == 0:
        return False

    N = V.size()
    mba = MBA(N)
    Y = mba.var('Y')
    G0 = Minv*Y.vec + Minv*V
    G1nl = Minv*NL(Y.vec)

    # Check if G1 is inversible through a dependency graph
    DG = nx.DiGraph()
    for i,e in enumerate(G1nl):
        for d in get_depends_as_set(e):
            DG.add_edge(Y[i].sym_idx(), d.sym_idx())
    
    return nx.is_directed_acyclic_graph(DG)

def app_inverse(A):
    M = A.matrix()
    NL = A.nl()
    V = A.cst()
    Minv = M.inverse()
    if Minv.ncols() == 0:
        return None

    N = V.size()
    mba = MBA(N)
    Y = mba.var('Y')
    G0 = Minv*Y.vec + Minv*V
    G1nl = simplify_inplace(Minv*NL(Y.vec))

    # Check if G1 is inversible through a dependency graph
    DG = nx.DiGraph()
    idx_base = Y[0].sym_idx()
    for i,e in enumerate(G1nl):
        for d in get_depends_as_set(e):
            DG.add_edge(Y[i].sym_idx()-idx_base, d.sym_idx()-idx_base)

    if not nx.is_directed_acyclic_graph(DG):
        return None

    # Use the graph to get the inverse of G1
    X = mba.var('X')
    resolve_order = reversed(list(nx.topological_sort(DG)))
    solved = dict()
    for i in resolve_order:
        e = G1nl[i]
        if e.is_imm() and e.imm_value() == 0:
            solved[i] = X[i]
        else:
            # Doing this in the reversed topological order should make this always work!
            solved[i] = simplify_inplace(X[i] + simplify_inplace(subs_exprs(G1nl[i], [Y[j] for j in six.iterkeys(solved)], list(six.itervalues(solved)))))
    G1inv = Vector(N)
    for i in range(N):
        G1inv[i] = solved.get(i, X[i])
    G1inv = MBAVariable(mba, G1inv)
    Finv = G1inv.eval({X: G0})
    #Finv = MBAVariable(mba, Finv)
    return Finv.vectorial_decomp([Y])

def find_one_esf(e, d):
    h = SymbolsHist()
    h.compute(e, d)
    #for v in h: print(v)

    # "Invert" the histogram
    hinv = collections.defaultdict(list)
    max_count = 0
    for v in h:
        c = v.count()
        hinv[c].append(v.sym())
        if max_count < c: max_count = c
    #print(hinv)

    # Compute the maximum number of arguments for this degree
    nterms = 0
    for a in e.args():
        if a.is_mul() and a.args().size() == d:
            nterms += 1
    max_args = solve_binomial(d, nterms)
    #print("max_args: %d" % max_args)

    # Compute the exact counts we will search from
    counts_args = filter(lambda v: v[0] <= max_count, ((int(binom(A-1,d-1)),A) for A in reversed(range(d+1,max_args+1))))

    for ca in counts_args:
        c,nargs = ca
        #print(d,c)
        cur_args = []
        for i in range(c,max_count+1):
            cur_args += hinv.get(i,[]) 
        if len(cur_args) < nargs:
            continue
        for A in reversed(range(nargs,max_args+1)):
            #print(d, "test %d args among %d (%d)" % (A, len(cur_args), binom(len(cur_args),A)))
            # TODO: optimize this
            for args in itertools.combinations(cur_args, A):
                args = sorted(args)
                test_esf = esf(d, list(args))
                anf_esf = simplify_inplace(expand_esf(test_esf))
                if e.contains(anf_esf):
                    #print("[+] found", test_esf)
                    return test_esf, anf_esf
    return None, None

def find_esfs_degree(e, d):
    esfs = []
    while True:
        esf, anf_esf = find_one_esf(e, d)
        if esf is None:
            break
        e += anf_esf
        simplify_inplace(e)
        esfs.append(esf)
    return esfs

def find_esfs(e):
    esfs = []
    max_degree = e.anf_esf_max_degree()
    #print("max_degree: %d" % max_degree)
    for d in reversed(range(2,max_degree+1)):
        esfs.extend(find_esfs_degree(e,d))
    for e_ in esfs: e += e_
    return esfs

def solve_binomial(k, v):
    # Find the biggest N such as (N,k) <= v by bruteforce
    # Trivial cases
    if (v == 0):
        return 0
    if (v == 1):
        return k
    N = k+2
    while True:
        t = int(binom(N,k))
        if t > v:
            return N-1
        N += 1
