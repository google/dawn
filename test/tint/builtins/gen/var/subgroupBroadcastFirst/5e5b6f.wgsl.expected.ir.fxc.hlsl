SKIP: FAILED


RWByteAddressBuffer prevent_dce : register(u0);
uint3 subgroupBroadcastFirst_5e5b6f() {
  uint3 arg_0 = (1u).xxx;
  uint3 res = WaveReadLaneFirst(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, subgroupBroadcastFirst_5e5b6f());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, subgroupBroadcastFirst_5e5b6f());
}

FXC validation failure:
C:\src\dawn\Shader@0x0000027736F50380(5,15-38): error X3004: undeclared identifier 'WaveReadLaneFirst'

