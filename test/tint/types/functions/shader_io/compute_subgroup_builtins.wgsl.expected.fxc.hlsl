SKIP: FAILED

RWByteAddressBuffer output : register(u0);

void main_inner(uint subgroup_invocation_id, uint subgroup_size) {
  output.Store((4u * subgroup_invocation_id), asuint(subgroup_size));
}

[numthreads(1, 1, 1)]
void main() {
  main_inner(WaveGetLaneIndex(), WaveGetLaneCount());
  return;
}
FXC validation failure:
C:\src\dawn\Shader@0x000001A2A5FE6E80(9,14-31): error X3004: undeclared identifier 'WaveGetLaneIndex'

