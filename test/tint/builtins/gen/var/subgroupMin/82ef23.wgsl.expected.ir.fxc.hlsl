SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint4 subgroupMin_82ef23() {
  uint4 arg_0 = (1u).xxxx;
  uint4 res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, subgroupMin_82ef23());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, subgroupMin_82ef23());
}

FXC validation failure:
C:\src\dawn\Shader@0x000001932213F560(5,15-34): error X3004: undeclared identifier 'WaveActiveMin'

