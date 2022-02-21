[numthreads(1, 1, 1)]
void main() {
  float3x3 m = float3x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  const float3 v = m[1];
  const float f = v[1];
  return;
}
