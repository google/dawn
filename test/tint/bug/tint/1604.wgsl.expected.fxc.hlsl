SKIP: FAILED

cbuffer cbuffer_x : register(b0, space0) {
  uint4 x[1];
};

[numthreads(1, 1, 1)]
void main() {
  switch(asint(x[0].x)) {
    case 0: {
      [loop] while (true) {
        return;
      }
      break;
    }
    default: {
      break;
    }
  }
  return;
}
FXC validation failure:
C:\src\dawn\test\tint\Shader@0x000001EA85F180E0(9,14-25): warning X3557: loop only executes for 0 iteration(s), consider removing [loop]
error X8000: D3D11 Internal Compiler Error: Invalid Bytecode: Can't fall through case/default unless case/default has no code. Opcode #9 (count 1-based). Aborting validation.
error X8000: D3D11 Internal Compiler Error: Invalid Bytecode: Can't continue validation - aborting.

