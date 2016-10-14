import copy
import arybo.lib.mba_exprs as EX

class ModifyingPass:
    def visit(self, e):
        return EX.visit(e, self)

    def visit_Expr(self, e):
        if e.args is None:
            return e
        args = [self.visit(a) for a in e.args]
        ret = copy.copy(e)
        ret.args = args
        return ret

class LowerRolRor(ModifyingPass):
    def visit_Rol(self, e):
        n = e.n
        arg = self.visit(e.arg)
        if n == 0:
            return arg
        nbits = arg.nbits
        return EX.ExprXor(
            EX.ExprShl(arg, n),
            EX.ExprLShr(arg, nbits-n))

    def visit_Ror(self, e):
        n = e.n
        arg = self.visit(e.arg)
        if n == 0:
            return arg
        nbits = arg.nbits
        return EX.ExprXor(
            EX.ExprShl(arg, nbits-n),
            EX.ExprLShr(arg, n))

def wrap_pass(cls):
    def f(e):
        P = cls()
        return EX.visit(e, P)
    return f

lower_rol_ror = wrap_pass(LowerRolRor)
