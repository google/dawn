
RWByteAddressBuffer output : register(u0);
void main_inner(uint subgroup_invocation_id, uint subgroup_size) {
  output.Store((0u + (uint(subgroup_invocation_id) * 4u)), subgroup_size);
}

void main() {
  uint v = WaveGetLaneIndex();
  main_inner(v, WaveGetLaneCount());
}

