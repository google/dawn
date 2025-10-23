struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared bool v;
int foo() {
  GroupMemoryBarrierWithGroupSync();
  bool v_1 = v;
  GroupMemoryBarrierWithGroupSync();
  if (v_1) {
    return int(42);
  }
  return int(0);
}

void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    v = false;
  }
  GroupMemoryBarrierWithGroupSync();
  foo();
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

