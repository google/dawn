RWTexture2D<uint4> x_20 : register(u1, space2);

void main_1() {
  const float f1 = 1.0f;
  const float2 vf12 = float2(1.0f, 2.0f);
  const float3 vf123 = float3(1.0f, 2.0f, 3.0f);
  const float4 vf1234 = float4(1.0f, 2.0f, 3.0f, 4.0f);
  const int i1 = 1;
  const int2 vi12 = int2(1, 2);
  const int3 vi123 = int3(1, 2, 3);
  const int4 vi1234 = int4(1, 2, 3, 4);
  const uint u1 = 1u;
  const uint2 vu12 = uint2(1u, 2u);
  const uint3 vu123 = uint3(1u, 2u, 3u);
  const uint4 vu1234 = uint4(1u, 2u, 3u, 4u);
  const int2 offsets2d = int2(3, 4);
  x_20[vi12] = vu1234;
  return;
}

void main() {
  main_1();
  return;
}
