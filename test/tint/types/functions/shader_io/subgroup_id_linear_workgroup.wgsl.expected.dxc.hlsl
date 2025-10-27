struct main_inputs {
  uint local_invocation_index : SV_GroupIndex;
};


RWByteAddressBuffer output : register(u0);
void main_inner(uint subgroup_id) {
  uint v = 0u;
  output.GetDimensions(v);
  output.Store((0u + (min(subgroup_id, ((v / 4u) - 1u)) * 4u)), subgroup_id);
}

[numthreads(64, 1, 1)]
void main(main_inputs inputs) {
  main_inner((inputs.local_invocation_index / WaveGetLaneCount()));
}

