RWByteAddressBuffer output : register(u0);

void main_inner(uint subgroup_invocation_id, uint subgroup_size) {
  output.Store((4u * subgroup_invocation_id), asuint(subgroup_size));
}

[numthreads(1, 1, 1)]
void main() {
  main_inner(WaveGetLaneIndex(), WaveGetLaneCount());
  return;
}
