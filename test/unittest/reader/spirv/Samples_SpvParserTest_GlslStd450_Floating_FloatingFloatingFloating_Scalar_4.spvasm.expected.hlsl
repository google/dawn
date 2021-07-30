void main_1() {
  const uint u1 = 10u;
  const uint u2 = 15u;
  const uint u3 = 20u;
  const int i1 = 30;
  const int i2 = 35;
  const int i3 = 40;
  const float f1 = 50.0f;
  const float f2 = 60.0f;
  const float f3 = 70.0f;
  const uint2 v2u1 = uint2(10u, 20u);
  const uint2 v2u2 = uint2(20u, 10u);
  const uint2 v2u3 = uint2(15u, 15u);
  const int2 v2i1 = int2(30, 40);
  const int2 v2i2 = int2(40, 30);
  const int2 v2i3 = int2(35, 35);
  const float2 v2f1 = float2(50.0f, 60.0f);
  const float2 v2f2 = float2(60.0f, 50.0f);
  const float2 v2f3 = float2(70.0f, 70.0f);
  const float3 v3f1 = float3(50.0f, 60.0f, 70.0f);
  const float3 v3f2 = float3(60.0f, 70.0f, 50.0f);
  const float4 v4f1 = float4(50.0f, 50.0f, 50.0f, 50.0f);
  const float4 v4f2 = v4f1;
  const float x_1 = smoothstep(f1, f2, f3);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
