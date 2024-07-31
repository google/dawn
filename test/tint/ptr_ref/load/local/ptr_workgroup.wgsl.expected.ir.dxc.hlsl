struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int i;
void main_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    i = 0;
  }
  GroupMemoryBarrierWithGroupSync();
  i = 123;
  int p = i;
  int u = (p + 1);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

