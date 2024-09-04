struct ComputeInputs {
  uint subgroup_invocation_id;
  uint subgroup_size;
};


RWByteAddressBuffer output : register(u0);
void main_inner(ComputeInputs inputs) {
  output.Store((0u + (uint(inputs.subgroup_invocation_id) * 4u)), inputs.subgroup_size);
}

[numthreads(1, 1, 1)]
void main() {
  uint v = WaveGetLaneIndex();
  ComputeInputs v_1 = {v, WaveGetLaneCount()};
  main_inner(v_1);
}

