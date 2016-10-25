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

# TODO: rename to tritonast2arybo
def triton2arybo(e, use_exprs=True, use_esf=False, context=None):
    ''' Convert a subset of Triton's AST into Arybo's representation

    Args:
        e: Triton AST
        use_esf: use ESFs when creating the final expression
        context: dictionnary that associates Triton expression ID to arybo expressions

    Returns:
        An :class:`arybo.lib.MBAVariable` object
    '''

    children_ = e.getChilds()
    children = (triton2arybo(c,use_exprs,use_esf,context) for c in children_)
    reversed_children = (triton2arybo(c,use_exprs,use_esf,context) for c in reversed(children_))

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
        name = e.getValue()
        ret = _get_mba(e.getBitvectorSize(),use_esf).var(name)
        if use_exprs:
            ret = EX.ExprBV(ret)
        return ret
    if Ty == TAstN.REFERENCE:
        if context is None:
            raise ValueError("reference node without context can't be resolved")
        id_ = e.getValue()
        ret = context.get(id_, None)
        if ret is None:
            raise ValueError("expression id %d not found in context" % id_)
        return ret

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
        if isinstance(n, (MBAVariable,EX.Expr)):
            n = n.to_cst()
        if not isinstance(n, six.integer_types):
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
    # Division is a special case because we only support division by a known
    # integer
    if Ty == TAstN.BVUDIV:
        a = next(children)
        n = next(children)
        if isinstance(n, (MBAVariable,EX.Expr)):
            n = n.to_cst()
        if not isinstance(n, six.integer_types):
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

tritonast2arybo = triton2arybo

def tritonexprs2arybo(exprs):
    context = {}
    e = None
    for id_,e in sorted(exprs.items()):
        if id_ in context:
            raise ValueError("expression id %d is set multiple times!" % id_)
        e = tritonast2arybo(e.getAst(), True, False, context)
        context[id_] = e
    return e
