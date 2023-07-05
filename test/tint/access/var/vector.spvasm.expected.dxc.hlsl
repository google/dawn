void main_1() {
  float3 v = (0.0f).xxx;
  const float x_14 = v.y;
  const float2 x_17 = v.xz;
  const float3 x_19 = v.xzy;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
