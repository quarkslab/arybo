import operator
import six
from six.moves import reduce

try:
    from triton import AST_NODE as TAstN
    triton_available = True
except ImportError:
    triton_available = False

from arybo.lib import MBA, MBAVariable, flatten
import arybo.lib.mba_exprs as EX

def _get_mba(n,use_esf):
    mba = MBA(n)
    mba.use_esf = use_esf
    return mba

def tritonast2arybo(e, use_exprs=True, use_esf=False, context=None):
    ''' Convert a subset of Triton's AST into Arybo's representation

    Args:
        e: Triton AST
        use_esf: use ESFs when creating the final expression
        context: dictionnary that associates Triton expression ID to arybo expressions

    Returns:
        An :class:`arybo.lib.MBAVariable` object
    '''

    children_ = e.getChildren()
    children = (tritonast2arybo(c,use_exprs,use_esf,context) for c in children_)
    reversed_children = (tritonast2arybo(c,use_exprs,use_esf,context) for c in reversed(children_))

    Ty = e.getType()
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
    if Ty == TAstN.INTEGER:
        return e.getInteger()
    if Ty == TAstN.BV:
        cst = next(children)
        nbits = next(children)
        if use_exprs:
            return EX.ExprCst(cst, nbits)
        else:
            return _get_mba(nbits,use_esf).from_cst(cst)
    if Ty == TAstN.EXTRACT:
        last = next(children)
        first = next(children)
        v = next(children)
        return v[first:last+1]
    if Ty == TAstN.CONCAT:
        if use_exprs:
            return EX.ExprConcat(*list(reversed_children))
        else:
            return flatten(reversed_children)
    if Ty == TAstN.VARIABLE:
        name = e.getSymbolicVariable().getName()
        ret = _get_mba(e.getBitvectorSize(),use_esf).var(name)
        if use_exprs:
            ret = EX.ExprBV(ret)
        return ret
    if Ty == TAstN.REFERENCE:
        if context is None:
            raise ValueError("reference node without context can't be resolved")
        id_ = e.getSymbolicExpression().getId()
        ret = context.get(id_, None)
        if ret is None:
            raise ValueError("expression id %d not found in context" % id_)
        return ret
    if Ty == TAstN.LET:
        # Alias
        # djo: "c'est pas utilise osef"
        raise ValueError("unsupported LET operation")

    # Logical/arithmetic shifts
    shifts = {
        TAstN.BVASHR: lambda a,b: a.ashr(b),
        TAstN.BVLSHR: lambda a,b: a.lshr(b),
        TAstN.BVSHL:  operator.lshift,
        TAstN.BVROL:  lambda x,n: x.rol(n),
        TAstN.BVROR:  lambda x,n: x.ror(n)
    }
    shift = shifts.get(Ty, None)
    if not shift is None:
        v = next(children)
        n = next(children)
        return shift(v,n)

    # Unary op
    unops = {
        TAstN.BVNOT: lambda x: ~x,
        TAstN.LNOT:  lambda x: ~x,
        TAstN.BVNEG: operator.neg
    }
    unop = unops.get(Ty, None)
    if not unop is None:
        return unop(next(children))

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
        TAstN.BVUDIV: lambda x,y: x.udiv(y),
        TAstN.BVSDIV: lambda x,y: x.sdiv(y),
        TAstN.BVUREM: lambda x,y: x.urem(y),
        TAstN.BVSREM: lambda x,y: x.srem(y),
        TAstN.LAND:   operator.and_,
        TAstN.LOR:    operator.or_
    }
    binop = binops.get(Ty, None)
    if not binop is None:
        return reduce(binop, children)

    # Logical op
    lops = {
        TAstN.EQUAL:    lambda x,y: EX.ExprCmpEq(x,y),
        TAstN.DISTINCT: lambda x,y: EX.ExprCmpNeq(x,y),
        TAstN.BVUGE:    lambda x,y: EX.ExprCmpGte(x,y,False),
        TAstN.BVUGT:    lambda x,y: EX.ExprCmpGt(x,y,False),
        TAstN.BVULE:    lambda x,y: EX.ExprCmpLte(x,y,False),
        TAstN.BVULT:    lambda x,y: EX.ExprCmpLt(x,y,False),
        TAstN.BVSGE:    lambda x,y: EX.ExprCmpGte(x,y,True),
        TAstN.BVSGT:    lambda x,y: EX.ExprCmpGt(x,y,True),
        TAstN.BVSLE:    lambda x,y: EX.ExprCmpLte(x,y,True),
        TAstN.BVSLT:    lambda x,y: EX.ExprCmpLt(x,y,True)
    }
    lop = lops.get(Ty, None)
    if not lop is None:
        return reduce(lop, children)

    # Conditional
    if Ty != TAstN.ITE:
        raise ValueError("unsupported node type %s" % str(Ty))
    return EX.ExprCond(next(children), next(children), next(children))

def tritonexprs2arybo(exprs):
    context = {}
    e = None
    for id_,e in sorted(exprs.items()):
        if id_ in context:
            raise ValueError("expression id %d is set multiple times!" % id_)
        e = tritonast2arybo(e.getAst(), True, False, context)
        context[id_] = e
    return e
