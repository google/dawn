SKIP: FAILED

void main_1() {
  uint u1 = 10u;
  uint u2 = 15u;
  uint u3 = 20u;
  int i1 = 30;
  int i2 = 35;
  int i3 = 40;
  float f1 = 0.5f;
  float f2 = 0.60000002384185791016f;
  float f3 = 0.69999998807907104492f;
  uint2 v2u1 = uint2(10u, 20u);
  uint2 v2u2 = uint2(20u, 10u);
  uint2 v2u3 = (15u).xx;
  int2 v2i1 = int2(30, 40);
  int2 v2i2 = int2(40, 30);
  int2 v2i3 = (35).xx;
  float2 v2f3 = (0.69999998807907104492f).xx;
  float3 v3f1 = float3(0.5f, 0.60000002384185791016f, 0.69999998807907104492f);
  float3 v3f2 = float3(0.60000002384185791016f, 0.69999998807907104492f, 0.5f);
  float4 v4f1 = (0.5f).xxxx;
  float4 v4f2 = v4f1;
  float2 x_1 = float2(0.65975391864776611328f, 0.77459669113159179688f);
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
}

