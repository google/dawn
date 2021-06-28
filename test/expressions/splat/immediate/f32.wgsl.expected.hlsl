[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float2 v2 = float2((1.0f).xx);
  float3 v3 = float3((1.0f).xxx);
  float4 v4 = float4((1.0f).xxxx);
}
