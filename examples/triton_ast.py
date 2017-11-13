import triton as TT 
from arybo.tools import tritonast2arybo

TT.setArchitecture(TT.ARCH.X86_64)

TT.convertRegisterToSymbolicVariable(TT.REG.RAX)
TT.convertRegisterToSymbolicVariable(TT.REG.RBX)

inst = TT.Instruction()
inst.setOpcodes("\x48\x31\xd8") # xor rax, rbx
TT.processing(inst)

rax_ast = TT.buildSymbolicRegister(TT.REG.RAX)
rax_ast = TT.getFullAst(rax_ast)
print(rax_ast)

e = tritonast2arybo(rax_ast)
print(e)
