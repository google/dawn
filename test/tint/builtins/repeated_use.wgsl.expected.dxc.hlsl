float4 tint_degrees(float4 param_0) {
  return param_0 * 57.29577951308232286465;
}

float3 tint_degrees_1(float3 param_0) {
  return param_0 * 57.29577951308232286465;
}

float2 tint_degrees_2(float2 param_0) {
  return param_0 * 57.29577951308232286465;
}

float tint_degrees_3(float param_0) {
  return param_0 * 57.29577951308232286465;
}

[numthreads(1, 1, 1)]
void main() {
  const float4 va = (0.0f).xxxx;
  const float4 a = tint_degrees(va);
  const float4 vb = (1.0f).xxxx;
  const float4 b = tint_degrees(vb);
  const float4 vc = float4(1.0f, 2.0f, 3.0f, 4.0f);
  const float4 c = tint_degrees(vc);
  const float3 vd = (0.0f).xxx;
  const float3 d = tint_degrees_1(vd);
  const float3 ve = (1.0f).xxx;
  const float3 e = tint_degrees_1(ve);
  const float3 vf = float3(1.0f, 2.0f, 3.0f);
  const float3 f = tint_degrees_1(vf);
  const float2 vg = (0.0f).xx;
  const float2 g = tint_degrees_2(vg);
  const float2 vh = (1.0f).xx;
  const float2 h = tint_degrees_2(vh);
  const float2 vi = float2(1.0f, 2.0f);
  const float2 i = tint_degrees_2(vi);
  const float vj = 1.0f;
  const float j = tint_degrees_3(vj);
  const float vk = 2.0f;
  const float k = tint_degrees_3(vk);
  const float vl = 3.0f;
  const float l = tint_degrees_3(vl);
  return;
}
