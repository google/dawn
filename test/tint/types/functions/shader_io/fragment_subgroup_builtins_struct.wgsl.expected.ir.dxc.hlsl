struct FragmentInputs {
  uint subgroup_invocation_id;
  uint subgroup_size;
};


RWByteAddressBuffer output : register(u0);
void main_inner(FragmentInputs inputs) {
  output.Store((0u + (uint(inputs.subgroup_invocation_id) * 4u)), inputs.subgroup_size);
}

void main() {
  FragmentInputs v = {WaveGetLaneIndex(), WaveGetLaneCount()};
  main_inner(v);
}

