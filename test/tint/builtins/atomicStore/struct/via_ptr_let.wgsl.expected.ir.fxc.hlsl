SKIP: FAILED

struct S {
  int x;
  uint a;
  uint y;
};

struct compute_main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared S wg;
void compute_main_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    wg.x = 0;
    uint v = 0u;
    InterlockedExchange(wg.a, 0u, v);
    wg.y = 0u;
  }
  GroupMemoryBarrierWithGroupSync();
  S p0 = wg;
  uint p1 = p0.a;
  uint v_1 = 0u;
  InterlockedExchange(p1, 1u, v_1);
}

[numthreads(1, 1, 1)]
void compute_main(compute_main_inputs inputs) {
  compute_main_inner(inputs.tint_local_index);
}

FXC validation failure:
c:\src\dawn\Shader@0x000001C9BA9ED340(24,3-34): error X3549: interlocked targets must be groupshared or UAV elements

