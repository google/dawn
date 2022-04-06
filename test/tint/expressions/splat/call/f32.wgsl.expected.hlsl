[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

float get_f32() {
  return 1.0f;
}

void f() {
  const float tint_symbol = get_f32();
  float2 v2 = float2((tint_symbol).xx);
  const float tint_symbol_1 = get_f32();
  float3 v3 = float3((tint_symbol_1).xxx);
  const float tint_symbol_2 = get_f32();
  float4 v4 = float4((tint_symbol_2).xxxx);
}
