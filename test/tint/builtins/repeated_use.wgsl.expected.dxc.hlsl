float4 tint_degrees(float4 param_0) {
  return param_0 * 57.295779513082322865;
}

float3 tint_degrees_1(float3 param_0) {
  return param_0 * 57.295779513082322865;
}

float2 tint_degrees_2(float2 param_0) {
  return param_0 * 57.295779513082322865;
}

float tint_degrees_3(float param_0) {
  return param_0 * 57.295779513082322865;
}

[numthreads(1, 1, 1)]
void main() {
  tint_degrees((0.0f).xxxx);
  tint_degrees((1.0f).xxxx);
  tint_degrees(float4(1.0f, 2.0f, 3.0f, 4.0f));
  tint_degrees_1((0.0f).xxx);
  tint_degrees_1((1.0f).xxx);
  tint_degrees_1(float3(1.0f, 2.0f, 3.0f));
  tint_degrees_2((0.0f).xx);
  tint_degrees_2((1.0f).xx);
  tint_degrees_2(float2(1.0f, 2.0f));
  tint_degrees_3(1.0f);
  tint_degrees_3(2.0f);
  tint_degrees_3(3.0f);
  return;
}
