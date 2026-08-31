struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int i;
void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    i = int(0);
  }
  GroupMemoryBarrierWithGroupSync();
  i = asint((asuint(i) + 1u));
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

