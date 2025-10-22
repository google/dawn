
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
    while((i < int(2))) {
      int v = i;
      float2 v_1 = v2f;
      float2 v_2 = float2((1.0f).xx);
      uint2 v_3 = uint2((uint(v)).xx);
      v2f = (((v_3 == uint2(0u, 1u))) ? (v_2) : (v_1));
      int v_4 = i;
      int2 v_5 = v2i;
      int2 v_6 = int2((int(1)).xx);
      uint2 v_7 = uint2((uint(v_4)).xx);
      v2i = (((v_7 == uint2(0u, 1u))) ? (v_6) : (v_5));
      int v_8 = i;
      uint2 v_9 = v2u;
      uint2 v_10 = uint2((1u).xx);
      uint2 v_11 = uint2((uint(v_8)).xx);
      v2u = (((v_11 == uint2(0u, 1u))) ? (v_10) : (v_9));
      int v_12 = i;
      bool2 v_13 = v2b;
      bool2 v_14 = bool2((true).xx);
      uint2 v_15 = uint2((uint(v_12)).xx);
      v2b = (((v_15 == uint2(0u, 1u))) ? (v_14) : (v_13));
      {
        i = asint((asuint(i) + asuint(int(1))));
      }
      continue;
    }
  }
  int i = int(0);
  int v_16 = i;
  float3 v_17 = v3f;
  float3 v_18 = float3((1.0f).xxx);
  uint3 v_19 = uint3((uint(v_16)).xxx);
  v3f = (((v_19 == uint3(0u, 1u, 2u))) ? (v_18) : (v_17));
  int v_20 = i;
  float4 v_21 = v4f;
  float4 v_22 = float4((1.0f).xxxx);
  uint4 v_23 = uint4((uint(v_20)).xxxx);
  v4f = (((v_23 == uint4(0u, 1u, 2u, 3u))) ? (v_22) : (v_21));
  int v_24 = i;
  int3 v_25 = v3i;
  int3 v_26 = int3((int(1)).xxx);
  uint3 v_27 = uint3((uint(v_24)).xxx);
  v3i = (((v_27 == uint3(0u, 1u, 2u))) ? (v_26) : (v_25));
  int v_28 = i;
  int4 v_29 = v4i;
  int4 v_30 = int4((int(1)).xxxx);
  uint4 v_31 = uint4((uint(v_28)).xxxx);
  v4i = (((v_31 == uint4(0u, 1u, 2u, 3u))) ? (v_30) : (v_29));
  int v_32 = i;
  uint3 v_33 = v3u;
  uint3 v_34 = uint3((1u).xxx);
  uint3 v_35 = uint3((uint(v_32)).xxx);
  v3u = (((v_35 == uint3(0u, 1u, 2u))) ? (v_34) : (v_33));
  int v_36 = i;
  uint4 v_37 = v4u;
  uint4 v_38 = uint4((1u).xxxx);
  uint4 v_39 = uint4((uint(v_36)).xxxx);
  v4u = (((v_39 == uint4(0u, 1u, 2u, 3u))) ? (v_38) : (v_37));
  int v_40 = i;
  bool3 v_41 = v3b;
  bool3 v_42 = bool3((true).xxx);
  uint3 v_43 = uint3((uint(v_40)).xxx);
  v3b = (((v_43 == uint3(0u, 1u, 2u))) ? (v_42) : (v_41));
  int v_44 = i;
  bool4 v_45 = v4b;
  bool4 v_46 = bool4((true).xxxx);
  uint4 v_47 = uint4((uint(v_44)).xxxx);
  v4b = (((v_47 == uint4(0u, 1u, 2u, 3u))) ? (v_46) : (v_45));
}

