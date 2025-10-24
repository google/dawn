struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


groupshared int a;
groupshared float4 b;
groupshared float2x2 c;
int tint_div_i32(int lhs, int rhs) {
  return (lhs / ((((rhs == int(0)) | ((lhs == int(-2147483648)) & (rhs == int(-1))))) ? (int(1)) : (rhs)));
}

void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    a = int(0);
    b = (0.0f).xxxx;
    c = float2x2((0.0f).xx, (0.0f).xx);
  }
  GroupMemoryBarrierWithGroupSync();
  a = tint_div_i32(a, int(2));
  b = mul(float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx), b);
  c = (c * 2.0f);
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

