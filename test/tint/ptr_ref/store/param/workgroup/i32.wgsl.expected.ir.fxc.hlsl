struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int S;
void func() {
  S = 42;
}

void main_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    S = 0;
  }
  GroupMemoryBarrierWithGroupSync();
  func();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

