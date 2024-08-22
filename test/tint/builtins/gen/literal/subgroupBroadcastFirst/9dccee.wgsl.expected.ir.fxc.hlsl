SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
int4 subgroupBroadcastFirst_9dccee() {
  int4 res = WaveReadLaneFirst((1).xxxx);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcastFirst_9dccee()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(subgroupBroadcastFirst_9dccee()));
}

FXC validation failure:
C:\src\dawn\Shader@0x0000016BE85DCE50(4,14-40): error X3004: undeclared identifier 'WaveReadLaneFirst'

