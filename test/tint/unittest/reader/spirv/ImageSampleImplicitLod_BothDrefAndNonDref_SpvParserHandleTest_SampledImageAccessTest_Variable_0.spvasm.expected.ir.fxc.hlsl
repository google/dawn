SKIP: FAILED


SamplerState x_10 : register(s0);
Texture2D x_20 : register(t1, space2);
SamplerComparisonState x_30 : register(s1);
void main_1() {
  float f1 = 1.0f;
  float2 vf12 = float2(1.0f, 2.0f);
  float2 vf21 = float2(2.0f, 1.0f);
  float3 vf123 = float3(1.0f, 2.0f, 3.0f);
  float4 vf1234 = float4(1.0f, 2.0f, 3.0f, 4.0f);
  int i1 = 1;
  int2 vi12 = int2(1, 2);
  int3 vi123 = int3(1, 2, 3);
  int4 vi1234 = int4(1, 2, 3, 4);
  uint u1 = 1u;
  uint2 vu12 = uint2(1u, 2u);
  uint3 vu123 = uint3(1u, 2u, 3u);
  uint4 vu1234 = uint4(1u, 2u, 3u, 4u);
  float coords1 = 1.0f;
  float2 coords12 = vf12;
  float3 coords123 = vf123;
  float4 coords1234 = vf1234;
  float4 x_200 = float4(x_20.Sample(x_10, coords12), 0.0f, 0.0f, 0.0f);
  float x_210 = x_20.SampleCmp(x_30, coords12, 0.20000000298023223877f);
}

void main() {
  main_1();
}

FXC validation failure:
<scrubbed_path>(23,18-70): error X3014: incorrect number of arguments to numeric-type constructor


tint executable returned error: exit status 1
