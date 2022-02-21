[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float a = float2((1.0f).xx).y;
  float b = float3((1.0f).xxx).z;
  float c = float4((1.0f).xxxx).w;
}
