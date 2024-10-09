
[numthreads(1, 1, 1)]
void main() {
  float2 v2f = (0.0f).xx;
  float3 v3f = (0.0f).xxx;
  float4 v4f = (0.0f).xxxx;
  int2 v2i = (int(0)).xx;
  int3 v3i = (int(0)).xxx;
  int4 v4i = (int(0)).xxxx;
  uint2 v2u = (0u).xx;
  uint3 v3u = (0u).xxx;
  uint4 v4u = (0u).xxxx;
  bool2 v2b = (false).xx;
  bool3 v3b = (false).xxx;
  bool4 v4b = (false).xxxx;
  {
    int i = int(0);
    while(true) {
      if ((i < int(2))) {
      } else {
        break;
      }
      float2 v = v2f;
      float2 v_1 = i.xx;
      v2f = (((v_1 == float2(int(0), int(1)))) ? (1.0f.xx) : (v));
      int2 v_2 = v2i;
      int2 v_3 = i.xx;
      v2i = (((v_3 == int2(int(0), int(1)))) ? (int(1).xx) : (v_2));
      uint2 v_4 = v2u;
      uint2 v_5 = i.xx;
      v2u = (((v_5 == uint2(int(0), int(1)))) ? (1u.xx) : (v_4));
      bool2 v_6 = v2b;
      bool2 v_7 = i.xx;
      v2b = (((v_7 == bool2(int(0), int(1)))) ? (true.xx) : (v_6));
      {
        i = (i + int(1));
      }
      continue;
    }
  }
  int i = int(0);
  float3 v_8 = v3f;
  float3 v_9 = i.xxx;
  v3f = (((v_9 == float3(int(0), int(1), int(2)))) ? (1.0f.xxx) : (v_8));
  float4 v_10 = v4f;
  float4 v_11 = i.xxxx;
  v4f = (((v_11 == float4(int(0), int(1), int(2), int(3)))) ? (1.0f.xxxx) : (v_10));
  int3 v_12 = v3i;
  int3 v_13 = i.xxx;
  v3i = (((v_13 == int3(int(0), int(1), int(2)))) ? (int(1).xxx) : (v_12));
  int4 v_14 = v4i;
  int4 v_15 = i.xxxx;
  v4i = (((v_15 == int4(int(0), int(1), int(2), int(3)))) ? (int(1).xxxx) : (v_14));
  uint3 v_16 = v3u;
  uint3 v_17 = i.xxx;
  v3u = (((v_17 == uint3(int(0), int(1), int(2)))) ? (1u.xxx) : (v_16));
  uint4 v_18 = v4u;
  uint4 v_19 = i.xxxx;
  v4u = (((v_19 == uint4(int(0), int(1), int(2), int(3)))) ? (1u.xxxx) : (v_18));
  bool3 v_20 = v3b;
  bool3 v_21 = i.xxx;
  v3b = (((v_21 == bool3(int(0), int(1), int(2)))) ? (true.xxx) : (v_20));
  bool4 v_22 = v4b;
  bool4 v_23 = i.xxxx;
  v4b = (((v_23 == bool4(int(0), int(1), int(2), int(3)))) ? (true.xxxx) : (v_22));
}

