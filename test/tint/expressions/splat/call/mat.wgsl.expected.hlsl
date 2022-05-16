[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

float get_f32() {
  return 1.0f;
}

float2x2 build_mat2x2(float value) {
  return float2x2(float2(value, value), float2(value, value));
}

float2x3 build_mat2x3(float value) {
  return float2x3(float3(value, value, value), float3(value, value, value));
}

float2x4 build_mat2x4(float value) {
  return float2x4(float4(value, value, value, value), float4(value, value, value, value));
}

float3x2 build_mat3x2(float value) {
  return float3x2(float2(value, value), float2(value, value), float2(value, value));
}

float3x3 build_mat3x3(float value) {
  return float3x3(float3(value, value, value), float3(value, value, value), float3(value, value, value));
}

float3x4 build_mat3x4(float value) {
  return float3x4(float4(value, value, value, value), float4(value, value, value, value), float4(value, value, value, value));
}

float4x2 build_mat4x2(float value) {
  return float4x2(float2(value, value), float2(value, value), float2(value, value), float2(value, value));
}

float4x3 build_mat4x3(float value) {
  return float4x3(float3(value, value, value), float3(value, value, value), float3(value, value, value), float3(value, value, value));
}

float4x4 build_mat4x4(float value) {
  return float4x4(float4(value, value, value, value), float4(value, value, value, value), float4(value, value, value, value), float4(value, value, value, value));
}

void f() {
  const float tint_symbol = get_f32();
  float2x2 m2x2 = build_mat2x2(tint_symbol);
  const float tint_symbol_1 = get_f32();
  float2x3 m2x3 = build_mat2x3(tint_symbol_1);
  const float tint_symbol_2 = get_f32();
  float2x4 m2x4 = build_mat2x4(tint_symbol_2);
  const float tint_symbol_3 = get_f32();
  float3x2 m3x2 = build_mat3x2(tint_symbol_3);
  const float tint_symbol_4 = get_f32();
  float3x3 m3x3 = build_mat3x3(tint_symbol_4);
  const float tint_symbol_5 = get_f32();
  float3x4 m3x4 = build_mat3x4(tint_symbol_5);
  const float tint_symbol_6 = get_f32();
  float4x2 m4x2 = build_mat4x2(tint_symbol_6);
  const float tint_symbol_7 = get_f32();
  float4x3 m4x3 = build_mat4x3(tint_symbol_7);
  const float tint_symbol_8 = get_f32();
  float4x4 m4x4 = build_mat4x4(tint_symbol_8);
}
