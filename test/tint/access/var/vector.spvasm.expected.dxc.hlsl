void main_1() {
  float3 v = (0.0f).xxx;
  const float x_14 = v.y;
  const float3 x_16 = v;
  const float2 x_17 = x_16.xz;
  const float3 x_18 = v;
  const float3 x_19 = x_18.xzy;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
