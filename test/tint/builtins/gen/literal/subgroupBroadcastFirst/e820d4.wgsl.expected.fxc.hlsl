SKIP: FAILED

RWByteAddressBuffer prevent_dce : register(u0);

int3 subgroupBroadcastFirst_e820d4() {
  int3 res = WaveReadLaneFirst((1).xxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcastFirst_e820d4()));
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(subgroupBroadcastFirst_e820d4()));
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x0000024FD9330390(4,14-39): error X3004: undeclared identifier 'WaveReadLaneFirst'

