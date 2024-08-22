SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupMax_15ccbf() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveMax(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupMax_15ccbf());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupMax_15ccbf());
}

FXC validation failure:
C:\src\dawn\Shader@0x00000282A9C14690(5,15-34): error X3004: undeclared identifier 'WaveActiveMax'

