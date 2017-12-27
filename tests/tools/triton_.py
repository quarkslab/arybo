import unittest
import random
import operator

from arybo.lib import MBA, simplify_inplace, expand_esf_inplace
from arybo.lib.mba_exprs import eval_expr
from arybo.lib.exprs_asm import to_llvm_function
from arybo.tools import tritonast2arybo, tritonexprs2arybo, triton_available

if triton_available:
    import triton as TT
    ctx = TT.TritonContext()
    ctx.setArchitecture(TT.ARCH.X86)
    TAst = ctx.getAstContext()

class TritonTest():
    def setUp(self):
        # Initialize the engine

        self.mba8  = MBA(8)
        self.mba16 = MBA(16)
        self.mba8.use_esf  = True
        self.mba16.use_esf = True
        # Useful variables using duringtests
        self.x8_t  = TAst.variable(ctx.newSymbolicVariable(8))
        self.y8_t  = TAst.variable(ctx.newSymbolicVariable(8))
        self.x16_t = TAst.variable(ctx.newSymbolicVariable(16))
        self.y16_t = TAst.variable(ctx.newSymbolicVariable(16))
        self.x8  = self.mba8.var(self.x8_t.getValue())
        self.y8  = self.mba8.var(self.y8_t.getValue())
        self.x16 = self.mba16.var(self.x16_t.getValue())
        self.y16 = self.mba16.var(self.y16_t.getValue())

    # Helpers
    def astEquals(self, ast, v):
        if self.use_expr:
            e = tritonast2arybo(ast,use_exprs=True,use_esf=False)
            e = eval_expr(e,use_esf=False)
            v = expand_esf_inplace(v)
            v = simplify_inplace(v)
        else:
            e = tritonast2arybo(ast,use_exprs=False,use_esf=True)
        self.assertEqual(e, v)

    # Tests
    def test_leaves(self):
        c  = random.choice(range(0xff))
        self.astEquals(
            TAst.bv(c, 8),
            self.mba8.from_cst(c))

        self.astEquals(
            self.x8_t,
            self.x8)

    def test_zx_sx(self):
        self.astEquals(
            TAst.zx(8, TAst.bv(0xff, 8)),
            self.mba16.from_cst(0x00ff))

        self.astEquals(
            TAst.sx(8, TAst.bv(0xff, 8)),
            self.mba16.from_cst(0xffff))

    def test_extract_contract(self):
        self.astEquals(
            TAst.extract(7, 0, TAst.bv(0xffff, 16)),
            self.mba8.from_cst(0xff))

        self.astEquals(
            TAst.concat([TAst.bv(0xff, 8), TAst.bv(0x00, 8)]),
            self.mba16.from_cst(0xff00))

    def test_unaryops(self):
        self.astEquals(
            TAst.bvnot(self.x8_t),
            ~self.x8)

        self.astEquals(
            TAst.bvneg(self.x8_t),
            -self.x8)

    def test_binaryops(self):
        ops = (
            (TAst.bvadd,  operator.add),
            (TAst.bvsub,  operator.sub),
            (TAst.bvand,  operator.and_),
            (TAst.bvor,   operator.or_),
            (TAst.bvxor,  operator.xor),
            (TAst.bvmul,  operator.mul),
            (TAst.bvnand, lambda x,y: ~(x&y)),
            (TAst.bvnor,  lambda x,y: ~(x|y)),
            (TAst.bvxnor, lambda x,y: ~(x^y))
        )
        for op in ops:
            self.astEquals(
                op[0](self.x8_t,self.y8_t),
                op[1](self.x8,self.y8))

        # One udiv test because it can take a lot of time...
        e = TAst.bvudiv(self.x8_t, TAst.bv(15, 8))
        self.astEquals(e, self.x8.udiv(15))

    def test_shifts(self):
        # Triton interface is not consistant between sh{r,l} and ro{l,r}.
        # For the first kind, it can take any AstNode. For the second one, an
        # integer is forced.
        ops = (
            (TAst.bvashr, lambda x,n: x.ashr(n)),
            (TAst.bvlshr, lambda x,n: x.lshr(n)),
            (TAst.bvshl,  operator.lshift),
        )
        for op in ops:
            for s in range(9):
                self.astEquals(
                    op[0](self.x8_t, TAst.bv(s, 8)),
                    op[1](self.x8, s))

        ops = (
            (TAst.bvrol,  lambda x,n: x.rol(n)),
            (TAst.bvror,  lambda x,n: x.ror(n))
        )
        for op in ops:
            for s in range(9):
                self.astEquals(
                    op[0](s, self.x8_t),
                    op[1](self.x8, s))

    def test_mba(self):
        # x^y = (x+y) - ((x&y)<<1)
        e = TAst.bvsub(
                TAst.bvadd(self.x8_t, self.y8_t),
                TAst.bvshl(TAst.bvand(self.x8_t, self.y8_t), TAst.bv(1, 8)))
        ea = tritonast2arybo(e,use_exprs=False,use_esf=True)
        simplify_inplace(expand_esf_inplace(ea))
        self.assertEqual(ea, self.x8^self.y8)

    def test_logical(self):
        e = TAst.equal(self.x8_t, self.y8_t)
        ea = tritonast2arybo(e,use_exprs=True,use_esf=False)
        ea = eval_expr(ea,use_esf=False)
        self.assertEqual(ea.nbits, 1)

    def test_exprs_xor_5C(self):
        # Based on djo's example
        
        # This is the xor_5C example compiled with optimisations for x86-4
        code = [
        "\x41\xB8\xE5\xFF\xFF\xFF",
        "\x89\xF8",
        "\xBA\x26\x00\x00\x00",
        "\x41\x0F\xAF\xC0",
        "\xB9\xED\xFF\xFF\xFF",
        "\x89\xC7",
        "\x89\xD0",
        "\x83\xEF\x09",
        "\x0F\xAF\xC7",
        "\x89\xC2",
        "\x89\xF8",
        "\x0F\xAF\xC1",
        "\xB9\x4B\x00\x00\x00",
        "\x8D\x44\x02\x2A",
        "\x0F\xB6\xC0",
        "\x89\xC2",
        "\xF7\xDA",
        "\x8D\x94\x12\xFF\x00\x00\x00",
        "\x81\xE2\xFE\x00\x00\x00",
        "\x01\xD0",
        "\x8D\x14\x00",
        "\x8D\x54\x02\x4D",
        "\x0F\xB6\xF2",
        "\x6B\xF6\x56",
        "\x83\xC6\x24",
        "\x83\xE6\x46",
        "\x89\xF0",
        "\x0F\xAF\xC1",
        "\xB9\xE7\xFF\xFF\xFF",
        "\x89\xC6",
        "\x89\xD0",
        "\xBA\x3A\x00\x00\x00",
        "\x0F\xAF\xC1",
        "\x89\xC1",
        "\x89\xD0",
        "\x8D\x4C\x0E\x76",
        "\xBE\x63\x00\x00\x00",
        "\x0F\xAF\xC1",
        "\x89\xC2",
        "\x89\xC8",
        "\x0F\xAF\xC6",
        "\x83\xEA\x51",
        "\xBE\x2D\x00\x00\x00",
        "\x83\xE2\xF4",
        "\x89\xC1",
        "\x8D\x4C\x0A\x2E",
        "\x89\xC8",
        "\x25\x94\x00\x00\x00",
        "\x01\xC0",
        "\x29\xC8",
        "\xB9\x67\x00\x00\x00",
        "\x0F\xAF\xC1",
        "\x8D\x48\x0D",
        "\x0F\xB6\xD1",
        "\x69\xD2\xAE\x00\x00\x00",
        "\x83\xCA\x22",
        "\x89\xD0",
        "\x41\x0F\xAF\xC0",
        "\x89\xC2",
        "\x89\xC8",
        "\x0F\xAF\xC6",
        "\x8D\x44\x02\xC2",
        "\x0F\xB6\xC0",
        "\x2D\xF7\x00\x00\x00",
        "\x69\xC0\xED\x00\x00\x00",
        "\x0F\xB6\xC0",
        ]

        ctx = TT.TritonContext()
        ctx.setArchitecture(TT.ARCH.X86_64)
        TAst = ctx.getAstContext()

        rdi = ctx.convertRegisterToSymbolicVariable(ctx.registers.rdi)
        rdi = tritonast2arybo(TAst.variable(rdi),use_exprs=False)

        for opcodes in code:
            inst = TT.Instruction(opcodes)
            ctx.processing(inst)

        rax_ast = ctx.buildSymbolicRegister(ctx.registers.rax)
        rax_ast = ctx.unrollAst(rax_ast)
        rax_ast = ctx.simplify(rax_ast, True)
        # Check that this gives a xor 5C
        e = tritonast2arybo(rax_ast,use_exprs=self.use_expr,use_esf=False)
        if self.use_expr:
            e = eval_expr(e)
        self.assertEqual(e, ((rdi&0xff)^0x5C).vec)

    def test_urem(self):
        code = [
        "\xbf\xAB\x00\x00\x00",     # mov   edi, 0xAB
        "\xf7\xff",                 # idiv  edi
        "\x83\xfa\x00"              # cmp   edx, 0x00
        ]

        ctx = TT.TritonContext()
        ctx.setArchitecture(TT.ARCH.X86_64)
        TAst = ctx.getAstContext()

        rax = ctx.convertRegisterToSymbolicVariable(ctx.registers.rax)
        rax = tritonast2arybo(TAst.variable(rax))
        rdx = ctx.convertRegisterToSymbolicVariable(ctx.registers.rdx)
        rdx = tritonast2arybo(TAst.variable(rdx))

        for opcodes in code:
            inst = TT.Instruction(opcodes)
            ctx.processing(inst)

        exprs = ctx.sliceExpressions(ctx.getSymbolicRegisters()[TT.REG.X86_64.RDX])
        e = tritonexprs2arybo(exprs)
        to_llvm_function(e, [rax.v, rdx.v])

@unittest.skipIf(triton_available == False, "skipping Triton-related tests as it is not available")
class TritonTestNoExpr(TritonTest, unittest.TestCase):
    def __init__(self, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)
        self.use_expr = False

@unittest.skipIf(triton_available == False, "skipping Triton-related tests as it is not available")
class TritonTestExpr(TritonTest, unittest.TestCase):
    def __init__(self, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)
        self.use_expr = True

if __name__ == "__main__":
    unittest.main()
