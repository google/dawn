
[numthreads(1, 1, 1)]
void main() {
  float2 v2f = (0.0f).xx;
  float2 v2f_2 = (0.0f).xx;
  int3 v3i = (int(0)).xxx;
  int3 v3i_2 = (int(0)).xxx;
  uint4 v4u = (0u).xxxx;
  uint4 v4u_2 = (0u).xxxx;
  bool2 v2b = (false).xx;
  bool2 v2b_2 = (false).xx;
  {
    int i = int(0);
    while((i < int(2))) {
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
      int v_16 = i;
      float2 v_17 = v2f_2;
      float2 v_18 = float2((1.0f).xx);
      uint2 v_19 = uint2((uint(v_16)).xx);
      v2f_2 = (((v_19 == uint2(0u, 1u))) ? (v_18) : (v_17));
      int v_20 = i;
      int3 v_21 = v3i_2;
      int3 v_22 = int3((int(1)).xxx);
      uint3 v_23 = uint3((uint(v_20)).xxx);
      v3i_2 = (((v_23 == uint3(0u, 1u, 2u))) ? (v_22) : (v_21));
      int v_24 = i;
      uint4 v_25 = v4u_2;
      uint4 v_26 = uint4((1u).xxxx);
      uint4 v_27 = uint4((uint(v_24)).xxxx);
      v4u_2 = (((v_27 == uint4(0u, 1u, 2u, 3u))) ? (v_26) : (v_25));
      int v_28 = i;
      bool2 v_29 = v2b_2;
      bool2 v_30 = bool2((true).xx);
      uint2 v_31 = uint2((uint(v_28)).xx);
      v2b_2 = (((v_31 == uint2(0u, 1u))) ? (v_30) : (v_29));
      {
        i = (i + int(1));
      }
      continue;
    }
  }
}

