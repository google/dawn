void ldexp_2cb32a() {
  float3 res = ldexp(float3(0.0f, 0.0f, 0.0f), uint3(0u, 0u, 0u));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  ldexp_2cb32a();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  ldexp_2cb32a();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_2cb32a();
  return;
}
