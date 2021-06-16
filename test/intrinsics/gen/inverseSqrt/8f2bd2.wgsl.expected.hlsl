void inverseSqrt_8f2bd2() {
  float2 res = rsqrt(float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  inverseSqrt_8f2bd2();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  inverseSqrt_8f2bd2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  inverseSqrt_8f2bd2();
  return;
}
