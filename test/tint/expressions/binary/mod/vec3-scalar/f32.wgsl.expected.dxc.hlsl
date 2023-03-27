float3 tint_trunc(float3 param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float3 tint_float_mod(float3 lhs, float rhs) {
  const float3 r = float3((rhs).xxx);
  return (lhs - (tint_trunc((lhs / r)) * r));
}

[numthreads(1, 1, 1)]
void f() {
  const float3 a = float3(1.0f, 2.0f, 3.0f);
  const float b = 4.0f;
  const float3 r = tint_float_mod(a, b);
  return;
}
