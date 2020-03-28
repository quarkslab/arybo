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

import six
import collections

import arybo.lib.mba_exprs as EX
from arybo.lib.exprs_passes import lower_rol_ror, CachePass

def IntType(n):
    return ll.IntType(int(n))

class ToLLVMIr(CachePass):
    def __init__(self, sym_to_value, IRB):
        super(ToLLVMIr,self).__init__()
        self.IRB = IRB
        self.sym_to_value = sym_to_value
        self.values = {}

    def visit_wrapper(self, e, cb):
        ret = super(ToLLVMIr, self).visit_wrapper(e, cb)
        if not isinstance(ret, tuple):
            return (ret,self.IRB.block)
        else:
            return ret

    def visit_value(self, e):
        return EX.visit(e, self)[0]

    def visit_Cst(self, e):
        return ll.Constant(IntType(e.nbits), e.n)

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
        return self.IRB.not_(self.visit_value(e.arg))

    def visit_ZX(self, e):
        return self.IRB.zext(self.visit_value(e.arg), IntType(e.n))

    def visit_SX(self, e):
        return self.IRB.sext(self.visit_value(e.arg), IntType(e.n))

    def visit_Concat(self, e):
        # Generate a suite of OR + shifts
        # TODO: pass that lowers concat
        arg0 = e.args[0]
        ret = self.visit_value(arg0)
        type_ = IntType(e.nbits)
        ret = self.IRB.zext(ret, type_)
        cur_bits = arg0.nbits
        for a in e.args[1:]:
            cur_arg = self.IRB.zext(self.visit_value(a), type_)
            ret = self.IRB.or_(ret,
                self.IRB.shl(cur_arg, ll.Constant(type_, cur_bits)))
            cur_bits += a.nbits
        return ret

    def visit_Slice(self, e):
        # TODO: pass that lowers slice
        ret = self.visit_value(e.arg)
        idxes = e.idxes
        # Support only sorted indxes for now
        if idxes != list(range(idxes[0], idxes[-1]+1)):
            raise ValueError("slice indexes must be continuous and sorted")
        if idxes[0] != 0:
            ret = self.IRB.lshr(ret, ll.Constant(IntType(e.arg.nbits), idxes[0]))
        return self.IRB.trunc(ret, IntType(len(idxes)))

    def visit_Broadcast(self, e):
        # TODO: pass that lowers broadcast
        # left-shift to get the idx as the MSB, and them use an arithmetic
        # right shift of nbits-1
        type_ = IntType(e.nbits)
        ret = self.visit_value(e.arg)
        ret = self.IRB.zext(ret, type_)
        ret = self.IRB.shl(ret, ll.Constant(type_, e.nbits-e.idx-1))
        return self.IRB.ashr(ret, ll.Constant(type_, e.nbits-1))

    def visit_nary_args(self, e, op):
        return op(*(self.visit_value(a) for a in e.args))

    def visit_BinaryOp(self, e):
        ops = {
            EX.ExprAdd: self.IRB.add,
            EX.ExprSub: self.IRB.sub,
            EX.ExprMul: self.IRB.mul,
            EX.ExprShl: self.IRB.shl,
            EX.ExprLShr: self.IRB.lshr,
            EX.ExprAShr: self.IRB.ashr
        }
        op = ops[type(e)]
        return self.visit_nary_args(e, op)

    def visit_Div(self, e):
        return self.visit_nary_args(e, self.IRB.sdiv if e.is_signed else self.IRB.udiv)

    def visit_Rem(self, e):
        return self.visit_nary_args(e, self.IRB.srem if e.is_signed else self.IRB.urem)

    def visit_NaryOp(self, e):
        ops = {
            EX.ExprXor: self.IRB.xor,
            EX.ExprAnd: self.IRB.and_,
            EX.ExprOr: self.IRB.or_,
        }
        op = ops[type(e)]
        return self.visit_nary_args(e, op)

    def visit_Cmp(self, e):
        f = self.IRB.icmp_signed if e.is_signed else self.IRB.icmp_unsigned
        cmp_op = {
            EX.ExprCmp.OpEq:  '==',
            EX.ExprCmp.OpNeq: '!=',
            EX.ExprCmp.OpLt:  '<',
            EX.ExprCmp.OpLte: '<=',
            EX.ExprCmp.OpGt:  '>',
            EX.ExprCmp.OpGte: '>='
        }
        return f(cmp_op[e.op], self.visit_value(e.X), self.visit_value(e.Y))

    def visit_Cond(self, e):
        cond = self.visit_value(e.cond)
        bb_name = self.IRB.basic_block.name
        ifb = self.IRB.append_basic_block(bb_name + ".if")
        elseb = self.IRB.append_basic_block(bb_name + ".else")
        endb = self.IRB.append_basic_block(bb_name + ".endif")
        self.IRB.cbranch(cond, ifb, elseb)

        self.IRB.position_at_end(ifb)
        ifv,ifb = EX.visit(e.a, self)
        self.IRB.branch(endb)

        self.IRB.position_at_end(elseb)
        elsev,elseb = EX.visit(e.b, self)
        self.IRB.branch(endb)

        self.IRB.position_at_end(endb)
        ret = self.IRB.phi(IntType(e.nbits))
        ret.add_incoming(ifv, ifb)
        ret.add_incoming(elsev, elseb)
        return ret,endb
    
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

def to_llvm_ir(exprs, sym_to_value, IRB):
    if not llvmlite_available:
        raise RuntimeError("llvmlite module unavailable! can't assemble to LLVM IR...")

    if not isinstance(exprs, collections.abc.Iterable):
        exprs = (exprs,)

    ret = None
    visitor = ToLLVMIr(sym_to_value, IRB)
    for e in exprs:
        e = lower_rol_ror(e)
        ret = visitor.visit_value(e)
    return ret

def to_llvm_function(exprs, vars_, name="__arybo"):
    if not llvmlite_available:
        raise RuntimeError("llvmlite module unavailable! can't assemble to LLVM IR...")

    if not isinstance(exprs, collections.abc.Iterable):
        exprs = (exprs,)

    M = ll.Module()
    args_types = [IntType(v.nbits) for v in vars_]
    fntype = ll.FunctionType(IntType(exprs[-1].nbits), args_types)
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
    ret = to_llvm_ir(exprs, sym_to_value, IRB)
    IRB.ret(ret)
    return M

def asm_module(exprs, dst_reg, sym_to_reg, triple_or_target=None):
    '''
    Generate an LLVM module for a list of expressions

    Arguments:
      * See :meth:`arybo.lib.exprs_asm.asm_binary` for a description of the list of arguments

    Output:
      * An LLVM module with one function named "__arybo", containing the
        translated expression.

    See :meth:`arybo.lib.exprs_asm.asm_binary` for an usage example.
    '''

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

    sym_to_value = {sym: IRB.load_reg(IntType(reg[1]), reg[0], reg[0]) for sym,reg in six.iteritems(sym_to_reg)}

    ret = to_llvm_ir(exprs, sym_to_value, IRB)
    IRB.store_reg(ret, IntType(dst_reg[1]), dst_reg[0])
    # See https://llvm.org/bugs/show_bug.cgi?id=15806
    IRB.unreachable()

    return M

def asm_binary(exprs, dst_reg, sym_to_reg, triple_or_target=None):
    '''
    Compile and assemble an expression for a given architecture.

    Arguments:
      * *exprs*: list of expressions to convert. This can represent a graph of
        expressions.
      * *dst_reg*: final register on which to store the result of the last
        expression. This is represented by a tuple ("reg_name", reg_size_bits).
        Example: ("rax", 64)
      * *sym_to_reg*: a dictionnary that maps Arybo variable name to registers
        (described as tuple, see *dst_reg*). Example: {"x": ("rdi",64), "y": ("rsi", 64)}
      * *triple_or_target*: LLVM architecture triple to use. Use by default the
        host architecture. Example: "x86_64-unknown-unknown"

    Output:
      * binary stream of the assembled expression for the given target

    Here is an example that will compile and assemble "x+y" for x86_64::

        from arybo.lib import MBA 
        from arybo.lib import mba_exprs
        from arybo.lib.exprs_asm import asm_binary
        mba = MBA(64)
        x = mba.var("x")
        y = mba.var("y")
        e = mba_exprs.ExprBV(x) + mba_exprs.ExprBV(y)
        code = asm_binary([e], ("rax", 64), {"x": ("rdi", 64), "y": ("rsi", 64)}, "x86_64-unknown-unknown")
        print(code.hex())

    which outputs ``488d0437`` (which is equivalent to ``lea rax,[rdi+rsi*1]``).
    '''
    if not llvmlite_available:
        raise RuntimeError("llvmlite module unavailable! can't assemble...")

    target = llvm_get_target(triple_or_target)
    M = asm_module(exprs, dst_reg, sym_to_reg, target)

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
