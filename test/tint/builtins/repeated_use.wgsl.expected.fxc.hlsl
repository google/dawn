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
  const float4 a = tint_degrees((0.0f).xxxx);
  const float4 b = tint_degrees((1.0f).xxxx);
  const float4 c = tint_degrees(float4(1.0f, 2.0f, 3.0f, 4.0f));
  const float3 d = tint_degrees_1((0.0f).xxx);
  const float3 e = tint_degrees_1((1.0f).xxx);
  const float3 f = tint_degrees_1(float3(1.0f, 2.0f, 3.0f));
  const float2 g = tint_degrees_2((0.0f).xx);
  const float2 h = tint_degrees_2((1.0f).xx);
  const float2 i = tint_degrees_2(float2(1.0f, 2.0f));
  const float j = tint_degrees_3(1.0f);
  const float k = tint_degrees_3(2.0f);
  const float l = tint_degrees_3(3.0f);
  return;
}
