import unittest
import operator
import six
import random

from six.moves import range,reduce
import ctypes
from ctypes import CFUNCTYPE

from arybo.lib.exprs_asm import llvmlite_available

if llvmlite_available:
    import llvmlite.binding as llvm

import arybo.lib.mba_exprs as EX
from arybo.lib import MBA
from arybo.lib.exprs_asm import to_llvm_function, llvm_get_target

def int_size_to_type(s):
    d = {
        8: ctypes.c_ubyte,
        16: ctypes.c_ushort,
        32: ctypes.c_uint,
        64: ctypes.c_ulonglong}
    return d[s]

@unittest.skipIf(llvmlite_available == False, "skipping LLVM-related tests as llvmlite is not available")
class LLVMTest(unittest.TestCase):
    def setUp(self):
        self.mba = MBA(8)
        self.x = self.mba.var('x')
        self.y = self.mba.var('y')
        self.ex = EX.ExprBV(self.x)
        self.ey = EX.ExprBV(self.y)
        self.args = [self.x,self.y]
        self.eargs = [EX.ExprBV(self.x),EX.ExprBV(self.y)]
        self.func_name = "__arybo"
        self.llvm_target = llvm_get_target()
        self.machine = self.llvm_target.create_target_machine()
        self.engine = llvm.create_mcjit_compiler(llvm.parse_assembly(""), self.machine)

    def get_c_func(self, e, args):
        # Get the llvm function
        M = to_llvm_function(e,self.args,self.func_name)
        # JIT the function, and compare random values
        M = llvm.parse_assembly(str(M))
        M.verify()
        self.engine.add_module(M)
        self.engine.finalize_object()

        func_ptr = self.engine.get_function_address(self.func_name)
        cfunc_type = (int_size_to_type(e.nbits),) + tuple(int_size_to_type(a.nbits) for a in args)
        cfunc = CFUNCTYPE(*cfunc_type)(func_ptr)
        return M,cfunc

    def check_expr(self, e, args):
        M,cfunc = self.get_c_func(e, args)
        # Eval 'e'
        evale = EX.eval_expr(e)
        for n in range(100):
            args_v = [random.getrandbits(a.nbits) for a in args]
            self.assertEqual(cfunc(*args_v), evale.eval({a: args_v[i] for i,a in enumerate(args)}))

        self.engine.remove_module(M)

    def test_tree(self):
        e0 = EX.ExprXor(self.ex, self.ey)
        e = EX.ExprAdd(e0,e0)
        self.check_expr(e, self.args)

    def test_binops(self):
        for op in (EX.ExprAdd,EX.ExprSub,EX.ExprMul,EX.ExprOr,EX.ExprXor,EX.ExprAnd):
            e = op(*self.eargs)
            self.check_expr(e, self.args)

    def test_shifts(self):
        for op in (EX.ExprShl,EX.ExprLShr,EX.ExprRor,EX.ExprRol):
            for n in range(8):
                e = op(self.ex, EX.ExprCst(n, 8))
                self.check_expr(e, [self.x])

    def test_concat_slice(self):
        e = EX.ExprConcat(self.ex[:4], self.ey[:4])
        self.check_expr(e, self.args)

        e = EX.ExprConcat(self.ex[:2], self.ey[2:8])
        self.check_expr(e, self.args)

    def test_broadcast(self):
        for nbits in (8,16):
            for i in range(8):
                e = EX.ExprBroadcast(self.ex, i, nbits)
                self.check_expr(e, [self.x])

    def test_not(self):
        e = EX.ExprNot(self.ex)
        self.check_expr(e, [self.x])

    def test_cond(self):
        e = EX.ExprCond(EX.ExprCmpEq(self.ex, EX.ExprCst(10, 8)), self.ex, self.ey)
        M,cfunc = self.get_c_func(e, self.args)
        for i in range(256):
            vref = 0xff if i != 10 else 10
            self.assertEqual(cfunc(i, 0xff), vref)
        self.engine.remove_module(M)

        e = EX.ExprCond(EX.ExprCmpGte(self.ex, EX.ExprCst(10, 8), is_signed=True), self.ex, self.ey)
        M,cfunc = self.get_c_func(e, self.args)
        for i in range(-128,128):
            vref = i if i >= 10 else 0xff
            self.assertEqual(cfunc(i, 0xff), vref)
        self.engine.remove_module(M)

if __name__ == "__main__":
    unittest.main()
