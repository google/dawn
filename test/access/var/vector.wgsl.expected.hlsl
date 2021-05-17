[numthreads(1, 1, 1)]
void main() {
  float3 v;
  const float scalar = v.y;
  const float2 swizzle2 = v.xz;
  const float3 swizzle3 = v.xzy;
  return;
}

