import operator
try:
    from triton import AST_NODE as TAstN
    triton_available = True
except ImportError:
    triton_available = False

from arybo.lib import MBA, MBAVariable, flatten

def _get_mba(n,use_esf):
    mba = MBA(n)
    mba.use_esf = use_esf
    return mba

def triton2arybo(e, use_esf=False):
    ''' Convert a subset of Triton's AST into Arybo's representation

    Args:
        e: Triton AST
        use_esf: use ESFs when creating the final expression

    Returns:
        An :class:`arybo.lib.MBAVariable` object
    '''

    children_ = e.getChilds()
    children = (triton2arybo(c,use_esf) for c in children_)
    reversed_children = (triton2arybo(c,use_esf) for c in reversed(children_))

    Ty = e.getKind()
    if Ty == TAstN.ZX:
        n = next(children)
        v = next(children)
        n += v.nbits
        if n == v.nbits:
            return v
        return v.zext(n)
    if Ty == TAstN.SX:
        n = next(children)
        v = next(children)
        n += v.nbits
        if n == v.nbits:
            return v
        return v.sext(n)
    if Ty == TAstN.DECIMAL:
        return e.getValue()
    if Ty == TAstN.BV:
        cst = next(children)
        nbits = next(children)
        return _get_mba(nbits,use_esf).from_cst(cst)
    if Ty == TAstN.EXTRACT:
        last = next(children)
        first = next(children)
        v = next(children)
        return v[first:last+1]
    if Ty == TAstN.CONCAT:
        return flatten(reversed_children)
    if Ty == TAstN.VARIABLE:
        name = e.getValue()
        return _get_mba(e.getBitvectorSize(),use_esf).var(name)

    # Logical/arithmetic shifts
    shifts = {
        TAstN.BVLSHR: operator.rshift,
        TAstN.BVSHL:  operator.lshift,
        TAstN.BVROL:  lambda x,n: x.rol(n),
        TAstN.BVROR:  lambda x,n: x.ror(n)
    }
    shift = shifts.get(Ty, None)
    if not shift is None:
        n = next(children)
        v = next(children)
        if isinstance(n, MBAVariable):
            n = n.to_cst()
        if not isinstance(n, (int,long)):
            raise ValueError("arithmetic/logical shifts by a symbolic value isn't supported yet.") 
        return shift(v,n)

    # Unary op
    unops = {
        TAstN.BVNOT: lambda x: ~x,
        TAstN.BVNEG: operator.neg
    }
    unop = unops.get(Ty, None)
    if unop != None:
        return unop(next(children))

    # Binary ops
    # Division is a special case because we only support division by a known
    # integer
    if Ty == TAstN.BVUDIV:
        a = next(children)
        n = next(children)
        if isinstance(n, MBAVariable):
            n = n.to_cst()
        if not isinstance(n, (int,long)):
            raise ValueError("unsigned division is only supported by a known integer!")
        return a.udiv(n)

    binops = {
        TAstN.BVADD:  operator.add,
        TAstN.BVSUB:  operator.sub,
        TAstN.BVAND:  operator.and_,
        TAstN.BVOR:   operator.or_,
        TAstN.BVXOR:  operator.xor,
        TAstN.BVMUL:  operator.mul,
        TAstN.BVNAND: lambda x,y: ~(x&y),
        TAstN.BVNOR:  lambda x,y: ~(x|y),
        TAstN.BVXNOR: lambda x,y: ~(x^y),
    }
    binop = binops[Ty]
    return reduce(binop, children)

