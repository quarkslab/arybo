import six
import operator
from six.moves import reduce

import arybo.lib.mba_exprs as EX
from arybo.lib import MBA, MBAVariable
from pytanque import Vector, Matrix, imm, App

def _identify_nonaffine(app,var_in):
    raise NotImplemented()

def __identify(app,in_name):
    # TODO: identify number of independant inputs
    NL = app.nl().vector()
    M = app.matrix()
    nbits_in = M.ncols()
    nbits_out = M.nlines()
    if nbits_in != nbits_out:
        raise ValueError("do not yet support application with a different\
                number of input and output bits!")
    mba = MBA(nbits_in)
    var_in = mba.var(in_name)

    if NL != Vector(len(NL)):
        return _identify_nonaffine(app,var_in)
    C = EX.ExprCst(mba.from_vec(app.cst()).to_cst(), nbits_out)
    if M == Matrix(nbits_out, nbits_in):
        # This is just a constant
        return C

    ret = EX.ExprBV(var_in)
    matrix_empty = 0
    # Find empty columns in the matrix.
    for j in range(nbits_in):
        is_zero = reduce(operator.and_, (M.at(i,j) == imm(0) for i in range(nbits_out)), True)
        if is_zero:
            matrix_empty |= 1<<j
    matrix_and_mask = (~matrix_empty)%(2**nbits_out)
    if matrix_empty != 0:
        ret = EX.ExprAnd(ret, EX.ExprCst(matrix_and_mask, nbits_out))
    if mba.from_vec(M*var_in.vec)^(var_in & matrix_empty) == var_in:
        # This is a XOR
        return EX.ExprXor(ret, C)
    raise ValueError("unable to identify an expression")

def identify(app_or_expr,in_):
    # type dispatching in python simply does not exist...
    if isinstance(app_or_expr, App):
        if not isinstance(in_,str):
            raise ValueError("invalid arguments for identify (expect (App,string))")
        return __identify(app_or_expr,in_)
    if isinstance(app_or_expr, Vector):
        app_or_expr = MBA(len(app_or_expr)).from_vec(app_or_expr)
    if not isinstance(app_or_expr, MBAVariable) or not isinstance(in_,MBAVariable) or in_.name is None:
        raise ValueError("invalid arguments for identify (expect (Vector|MBAVariable, MBAVariable))")
    app = app_or_expr.vectorial_decomp([in_])
    return __identify(app, in_.name)
