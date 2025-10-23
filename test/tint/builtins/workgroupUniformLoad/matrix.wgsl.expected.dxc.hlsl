struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared float3x3 v;
float3x3 foo() {
  GroupMemoryBarrierWithGroupSync();
  float3x3 v_1 = v;
  GroupMemoryBarrierWithGroupSync();
  return v_1;
}

void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  }
  GroupMemoryBarrierWithGroupSync();
  foo();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

