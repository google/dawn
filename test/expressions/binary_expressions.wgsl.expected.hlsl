void bitwise_i32() {
  int s1 = 0;
  int s2 = 0;
  int3 v1 = int3(0, 0, 0);
  int3 v2 = int3(0, 0, 0);
  s1 = (s1 | s2);
  s1 = (s1 & s2);
  s1 = (s1 ^ s2);
  v1 = (v1 | v2);
  v1 = (v1 & v2);
  v1 = (v1 ^ v2);
}

void bitwise_u32() {
  uint s1 = 0u;
  uint s2 = 0u;
  uint3 v1 = uint3(0u, 0u, 0u);
  uint3 v2 = uint3(0u, 0u, 0u);
  s1 = (s1 | s2);
  s1 = (s1 & s2);
  s1 = (s1 ^ s2);
  v1 = (v1 | v2);
  v1 = (v1 & v2);
  v1 = (v1 ^ v2);
}

void vector_scalar_f32() {
  float3 v = float3(0.0f, 0.0f, 0.0f);
  float s = 0.0f;
  float3 r = float3(0.0f, 0.0f, 0.0f);
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
}

void vector_scalar_i32() {
  int3 v = int3(0, 0, 0);
  int s = 0;
  int3 r = int3(0, 0, 0);
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
  r = (v % s);
}

void vector_scalar_u32() {
  uint3 v = uint3(0u, 0u, 0u);
  uint s = 0u;
  uint3 r = uint3(0u, 0u, 0u);
  r = (v + s);
  r = (v - s);
  r = (v * s);
  r = (v / s);
  r = (v % s);
}

void scalar_vector_f32() {
  float3 v = float3(0.0f, 0.0f, 0.0f);
  float s = 0.0f;
  float3 r = float3(0.0f, 0.0f, 0.0f);
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
}

void scalar_vector_i32() {
  int3 v = int3(0, 0, 0);
  int s = 0;
  int3 r = int3(0, 0, 0);
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
  r = (s % v);
}

void scalar_vector_u32() {
  uint3 v = uint3(0u, 0u, 0u);
  uint s = 0u;
  uint3 r = uint3(0u, 0u, 0u);
  r = (s + v);
  r = (s - v);
  r = (s * v);
  r = (s / v);
  r = (s % v);
}

void matrix_matrix_f32() {
  float3x4 m34 = float3x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x3 m43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3x3 m33 = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x4 m44 = float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  m34 = (m34 + m34);
  m34 = (m34 - m34);
  m33 = mul(m34, m43);
  m44 = mul(m43, m34);
}

struct tint_symbol {
  float4 value : SV_Target0;
};

tint_symbol main() {
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}
