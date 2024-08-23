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

DXC validation failure:
hlsl.hlsl:14:25: warning: equality comparison with extraneous parentheses [-Wparentheses-equality]
  if ((tint_local_index == 0u)) {
       ~~~~~~~~~~~~~~~~~^~~~~
hlsl.hlsl:14:25: note: remove extraneous parentheses around the comparison to silence this warning
  if ((tint_local_index == 0u)) {
      ~                 ^    ~
hlsl.hlsl:14:25: note: use '=' to turn this equality comparison into an assignment
  if ((tint_local_index == 0u)) {
                        ^~
                        =
error: cannot map resource to handle.
hlsl.hlsl:24:3: error: Atomic operation targets must be groupshared, Node Record or UAV.
  InterlockedExchange(p1, 1u, v_1);
  ^


tint executable returned error: exit status 1
