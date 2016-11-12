import idc
import idaapi
import time

from rpyc import classic
c = classic.connect("127.0.0.1",port=18812)
 
triton = c.modules.triton
tast = c.modules['triton.ast']
aexprs = c.modules['arybo.lib.mba_exprs']
easm = c.modules['arybo.lib.exprs_asm']
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
app = e.vectorial_decomp([rdi.v])
exp = atools.identify(app,"rdi")
print("[*] Identified expression: rax = %s" % aexprs.prettyprint(exp))
asm = easm.asm_binary(exp, ("rax",64), {"rdi": ("rdi",64)}, "x86_64-unknown-unknwon")
print("[*] Assembled expression: %s" % asm.encode("hex"))

func_size = func.endEA-1-func.startEA
if len(asm) > func_size:
    printf("[-] Final assembly does not fit in the original function!")
asm_nop = asm + "\x90"*(func_size-len(asm))
func_start = int(func.startEA)
func_end = int(func.endEA)
idaapi.patch_many_bytes(func_start, asm_nop)
idaapi.do_unknown_range(func_start, func_end, 0)
idaapi.auto_make_code(func_start)
idaapi.auto_make_proc(func_start)
