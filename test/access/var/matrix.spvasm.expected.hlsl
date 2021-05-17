[numthreads(1, 1, 1)]
void main() {
  float3x3 m = float3x3(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
  const float3 x_15 = m[1];
  const float x_16 = x_15.y;
  return;
}

