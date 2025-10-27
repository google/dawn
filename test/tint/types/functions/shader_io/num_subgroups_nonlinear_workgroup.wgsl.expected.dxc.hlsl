
RWByteAddressBuffer output : register(u0);
groupshared uint tint_subgroup_id_counter;
void main_inner(uint num_subgroups) {
  uint v = 0u;
  output.GetDimensions(v);
  output.Store((0u + (min(num_subgroups, ((v / 4u) - 1u)) * 4u)), num_subgroups);
}

[numthreads(8, 8, 1)]
void main() {
  uint v_1 = 0u;
  InterlockedExchange(tint_subgroup_id_counter, 0u, v_1);
  GroupMemoryBarrierWithGroupSync();
  uint tint_subgroup_id = 0u;
  if (WaveIsFirstLane()) {
    uint v_2 = 0u;
    InterlockedAdd(tint_subgroup_id_counter, 1u, v_2);
    tint_subgroup_id = v_2;
  }
  GroupMemoryBarrierWithGroupSync();
  uint v_3 = 0u;
  InterlockedOr(tint_subgroup_id_counter, 0u, v_3);
  main_inner(v_3);
}

