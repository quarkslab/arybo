import idc
import idaapi
import time

from rpyc import classic
c = classic.connect("127.0.0.1",port=18812)
 
triton = c.modules.triton
tast = c.modules['triton.ast']
aexprs = c.modules['arybo.lib.mba_exprs']
atools = c.modules['arybo.tools']
triton.setArchitecture(triton.ARCH.X86_64)
#triton.setAstRepresentationMode(triton.AST_REPRESENTATION.PYTHON)
#triton.enableSymbolicOptimization(triton.OPTIMIZATION.ALIGNED_MEMORY, True)
 
sym_rdi = triton.convertRegisterToSymbolicVariable(triton.REG.RDI, "rdi input")
rdi = atools.tritonast2arybo(tast.variable(sym_rdi))
print("[ ] %s = RDI" % str(sym_rdi))

ea = idc.ScreenEA()
func = idaapi.get_func(ea)

pc = func.startEA
print("[+] computing Triton AST for function starting at 0x%x, ending at 0x%x..." % (func.startEA, func.endEA))
while pc < func.endEA-1:
    inst = triton.Instruction()
    opcode = idc.GetManyBytes(pc, idc.ItemSize(pc))
    inst.setOpcodes(opcode)
    inst.setAddress(pc)
    triton.processing(inst)
    pc = triton.getSymbolicRegisterValue(triton.REG.RIP)
 
rax_ast = triton.buildSymbolicRegister(triton.REG.RAX)
rax_ast = triton.getFullAst(rax_ast)
rax_ast = triton.simplify(rax_ast, True)
start = time.time()
print("[+] computing Arybo representation...")
e = atools.tritonast2arybo(rax_ast,use_exprs=True,use_esf=False)
print("[+] got Arybo expression, evalute it...")
e = aexprs.eval_expr(e,use_esf=False)
end = time.time()
diff = end-start
print("[*] Arybo evaluation computed in %0.4fs" % diff)
print("[*] RAX:")
print(e)
