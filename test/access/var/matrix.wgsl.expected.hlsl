[numthreads(1, 1, 1)]
void main() {
  float3x3 m;
  const float3 v = m[1];
  const float f = v[1];
  return;
}

