import idc
import idaapi

from rpyc import classic
c = classic.connect("127.0.0.1",port=18812)
 
triton = c.modules.triton
tast = c.modules['triton.ast']
atools = c.modules['arybo.tools']
triton.setArchitecture(triton.ARCH.X86_64)
triton.setAstRepresentationMode(triton.AST_REPRESENTATION.PYTHON)
 
sym_rdi = triton.convertRegisterToSymbolicVariable(triton.REG.RDI, "rdi input")
rdi = atools.triton2arybo(tast.variable(sym_rdi))
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
print("[+] computing Arybo representation...")
e = atools.triton2arybo(rax_ast,use_esf=False)
print("[*] RAX:")
print(e)
