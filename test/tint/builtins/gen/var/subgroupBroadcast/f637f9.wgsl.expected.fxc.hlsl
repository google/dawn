SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int4 subgroupBroadcast_f637f9() {
  int4 arg_0 = (1).xxxx;
  int4 res = WaveReadLaneAt(arg_0, 1u);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcast_f637f9()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x00000247A6AEF3F0(5,14-38): error X3004: undeclared identifier 'WaveReadLaneAt'

