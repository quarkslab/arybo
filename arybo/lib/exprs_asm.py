# Assemble Arybo IR into ASM thanks to LLVM
# Map symbol names to register
# Warning: this tries to do its best to save modified temporary registers.
# There might be errors while doing this. The idea is not to regenerate clean
# binaries, but to help the reverser!

try:
    import llvmlite.ir as ll
    import llvmlite.binding as llvm
    import ctypes
    llvmlite_available = True
    __llvm_initialized = False
except ImportError:
    llvmlite_available = False

import arybo.lib.mba_exprs as EX
from arybo.lib.exprs_passes import lower_rol_ror, CachePass
import six

class ToLLVMIr(CachePass):
    def __init__(self, sym_to_value, IRB):
        super(ToLLVMIr,self).__init__()
        self.IRB = IRB
        self.sym_to_value = sym_to_value
        self.values = {}

    def visit_Cst(self, e):
        return ll.Constant(ll.IntType(e.nbits), e.n)

    def visit_BV(self, e):
        name = e.v.name
        value = self.sym_to_value.get(name, None)
        if value is None:
            raise ValueError("unable to map BV name '%s' to an LLVM value!" % name)
        # TODO: check value bit-size
        #ret,nbits = value
        #if e.nbits != nbits:
        #    raise ValueError("bit-vector is %d bits, expected %d bits" % (e.nbits, nbits))
        return value

    def visit_Not(self, e):
        return self.IRB.not_(EX.visit(e.arg, self))

    def visit_UnaryOp(self, e):
        ops = {
            EX.ExprShl: self.IRB.shl,
            EX.ExprLShr: self.IRB.lshr,
        }
        op = ops[type(e)]
        return op(EX.visit(e.arg, self), ll.Constant(ll.IntType(e.nbits), e.n))

    def visit_ZX(self, e):
        return self.IRB.zext(EX.visit(e.arg, self), ll.IntType(e.n))

    def visit_SX(self, e):
        return self.IRB.sext(EX.visit(e.arg, self), ll.IntType(e.n))

    def visit_Concat(self, e):
        # Generate a suite of OR + shifts
        # TODO: pass that lowers concat
        arg0 = e.args[0]
        ret = EX.visit(arg0, self)
        type_ = ll.IntType(e.nbits)
        ret = self.IRB.zext(ret, type_)
        cur_bits = arg0.nbits
        for a in e.args[1:]:
            cur_arg = self.IRB.zext(EX.visit(a, self), type_)
            ret = self.IRB.or_(ret,
                self.IRB.shl(cur_arg, ll.Constant(type_, cur_bits)))
            cur_bits += a.nbits
        return ret

    def visit_Slice(self, e):
        # TODO: pass that lowers slice
        ret = EX.visit(e.arg, self)
        idxes = e.idxes
        # Support only sorted indxes for now
        if idxes != list(range(idxes[0], idxes[-1]+1)):
            raise ValueError("slice indexes must be continuous and sorted")
        if idxes[0] != 0:
            ret = self.IRB.lshr(ret, ll.Constant(ll.IntType(e.arg.nbits), idxes[0]))
        return self.IRB.trunc(ret, ll.IntType(len(idxes)))

    def visit_Broadcast(self, e):
        # TODO: pass that lowers broadcast
        # left-shift to get the idx as the MSB, and them use an arithmetic
        # right shift of nbits-1
        type_ = ll.IntType(e.nbits)
        ret = EX.visit(e.arg, self)
        ret = self.IRB.zext(ret, type_)
        ret = self.IRB.shl(ret, ll.Constant(type_, e.nbits-e.idx-1))
        return self.IRB.ashr(ret, ll.Constant(type_, e.nbits-1))

    def visit_nary_args(self, e, op):
        return op(*(EX.visit(a, self) for a in e.args))

    def visit_BinaryOp(self, e):
        ops = {
            EX.ExprAdd: self.IRB.add,
            EX.ExprSub: self.IRB.sub,
            EX.ExprMul: self.IRB.mul,
            EX.ExprDiv: self.IRB.udiv
        }
        op = ops[type(e)]
        return self.visit_nary_args(e, op)

    def visit_NaryOp(self, e):
        ops = {
            EX.ExprXor: self.IRB.xor,
            EX.ExprAnd: self.IRB.and_,
            EX.ExprOr: self.IRB.or_,
        }
        op = ops[type(e)]
        return self.visit_nary_args(e, op)
    
def llvm_get_target(triple_or_target=None):
    global __llvm_initialized
    if not __llvm_initialized:
        # Lazy initialisation
        llvm.initialize()
        llvm.initialize_all_targets()
        llvm.initialize_all_asmprinters()
        __llvm_initialized = True

    if isinstance(triple_or_target, llvm.Target):
        return triple_or_target
    if triple_or_target is None:
        return llvm.Target.from_default_triple()
    return llvm.Target.from_triple(triple_or_target)

def _create_execution_engine(M, target):
    target_machine = target.create_target_machine()
    engine = llvm.create_mcjit_compiler(M, target_machine)
    return engine

def to_llvm_ir(expr, sym_to_value, IRB):
    if not llvmlite_available:
        raise RuntimeError("llvmlite module unavailable! can't assemble to LLVM IR...")

    expr = lower_rol_ror(expr)
    visitor = ToLLVMIr(sym_to_value, IRB)
    return EX.visit(expr, visitor)

def to_llvm_function(expr, vars_, name="__arybo"):
    if not llvmlite_available:
        raise RuntimeError("llvmlite module unavailable! can't assemble to LLVM IR...")

    M = ll.Module()
    args_types = [ll.IntType(v.nbits) for v in vars_]
    fntype = ll.FunctionType(ll.IntType(expr.nbits), args_types)
    func = ll.Function(M, fntype, name=name)
    func.attributes.add("nounwind")
    BB = func.append_basic_block()

    IRB = ll.IRBuilder()
    IRB.position_at_end(BB)

    sym_to_value = {}
    for i,v in enumerate(vars_):
        arg = func.args[i]
        arg.name = v.name
        sym_to_value[v.name] = arg
    ret = to_llvm_ir(expr, sym_to_value, IRB)
    IRB.ret(ret)
    return M

def asm_module(expr, dst_reg, sym_to_reg, triple_or_target=None):
    if not llvmlite_available:
        raise RuntimeError("llvmlite module unavailable! can't assemble...")

    target = llvm_get_target(triple_or_target)

    M = ll.Module()
    fntype = ll.FunctionType(ll.VoidType(), [])
    func = ll.Function(M, fntype, name='__arybo')
    func.attributes.add("naked")
    func.attributes.add("nounwind")
    BB = func.append_basic_block()

    IRB = ll.IRBuilder()
    IRB.position_at_end(BB)

    sym_to_value = {sym: IRB.load_reg(reg[1], reg[0], reg[0]) for sym,reg in six.iteritems(sym_to_reg)}

    ret = to_llvm_ir(expr, sym_to_value, IRB)
    IRB.store_reg(ret, dst_reg[1], dst_reg[0])
    # See https://llvm.org/bugs/show_bug.cgi?id=15806
    IRB.unreachable()

    return M

def asm_binary(expr, dst_reg, sym_to_reg, triple_or_target=None):
    if not llvmlite_available:
        raise RuntimeError("llvmlite module unavailable! can't assemble...")

    target = llvm_get_target(triple_or_target)
    M = asm_module(expr, dst_reg, sym_to_reg, target)

    # Use LLVM to compile the '__arybo' function. As the function is naked and
    # is the only, we just got to dump the .text section to get the binary
    # assembly.
    # No need for keystone or whatever hype stuff. llvmlite does the job.

    M = llvm.parse_assembly(str(M))
    M.verify()
    target_machine = target.create_target_machine()
    obj_bin = target_machine.emit_object(M)
    obj = llvm.ObjectFileRef.from_data(obj_bin)
    for s in obj.sections():
        if s.is_text():
            return s.data()
    raise RuntimeError("unable to get the assembled binary!")
