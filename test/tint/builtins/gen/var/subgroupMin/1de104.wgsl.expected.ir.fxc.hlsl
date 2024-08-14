SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupMin_1de104() {
  int4 arg_0 = (1).xxxx;
  int4 res = WaveActiveMin(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_1de104()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupMin_1de104()));
}

FXC validation failure:
C:\src\dawn\Shader@0x000001DFB4F68FF0(5,14-33): error X3004: undeclared identifier 'WaveActiveMin'

