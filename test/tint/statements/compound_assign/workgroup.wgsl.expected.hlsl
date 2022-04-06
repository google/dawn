[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

groupshared int a;
groupshared float4 b;
groupshared float2x2 c;

void foo() {
  a = (a / 2);
  b = mul(float4x4(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f), b);
  c = (c * 2.0f);
}
