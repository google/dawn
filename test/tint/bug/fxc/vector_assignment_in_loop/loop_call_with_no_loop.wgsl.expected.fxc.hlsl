
static float2 v2f = (0.0f).xx;
static int3 v3i = (int(0)).xxx;
static uint4 v4u = (0u).xxxx;
static bool2 v2b = (false).xx;
void foo() {
  int i = int(0);
  int v = i;
  float2 v_1 = v2f;
  float2 v_2 = float2((1.0f).xx);
  uint2 v_3 = uint2((uint(v)).xx);
  v2f = (((v_3 == uint2(0u, 1u))) ? (v_2) : (v_1));
  int v_4 = i;
  int3 v_5 = v3i;
  int3 v_6 = int3((int(1)).xxx);
  uint3 v_7 = uint3((uint(v_4)).xxx);
  v3i = (((v_7 == uint3(0u, 1u, 2u))) ? (v_6) : (v_5));
  int v_8 = i;
  uint4 v_9 = v4u;
  uint4 v_10 = uint4((1u).xxxx);
  uint4 v_11 = uint4((uint(v_8)).xxxx);
  v4u = (((v_11 == uint4(0u, 1u, 2u, 3u))) ? (v_10) : (v_9));
  int v_12 = i;
  bool2 v_13 = v2b;
  bool2 v_14 = bool2((true).xx);
  uint2 v_15 = uint2((uint(v_12)).xx);
  v2b = (((v_15 == uint2(0u, 1u))) ? (v_14) : (v_13));
}

[numthreads(1, 1, 1)]
void main() {
  {
    int i = int(0);
    while((i < int(2))) {
      foo();
      {
        i = asint((asuint(i) + asuint(int(1))));
      }
      continue;
    }
  }
}

