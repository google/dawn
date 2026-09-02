
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
    for( ; (i < int(2)); i = asint((asuint(i) + 1u))) {
      float2 v = v2f;
      v2f = (((uint2((uint(i)).xx) == uint2(0u, 1u))) ? ((1.0f).xx) : (v));
      int3 v_1 = v3i;
      v3i = (((uint3((uint(i)).xxx) == uint3(0u, 1u, 2u))) ? ((int(1)).xxx) : (v_1));
      uint4 v_2 = v4u;
      v4u = (((uint4((uint(i)).xxxx) == uint4(0u, 1u, 2u, 3u))) ? ((1u).xxxx) : (v_2));
      bool2 v_3 = v2b;
      v2b = (((uint2((uint(i)).xx) == uint2(0u, 1u))) ? ((true).xx) : (v_3));
      float2 v_4 = v2f_2;
      v2f_2 = (((uint2((uint(i)).xx) == uint2(0u, 1u))) ? ((1.0f).xx) : (v_4));
      int3 v_5 = v3i_2;
      v3i_2 = (((uint3((uint(i)).xxx) == uint3(0u, 1u, 2u))) ? ((int(1)).xxx) : (v_5));
      uint4 v_6 = v4u_2;
      v4u_2 = (((uint4((uint(i)).xxxx) == uint4(0u, 1u, 2u, 3u))) ? ((1u).xxxx) : (v_6));
      bool2 v_7 = v2b_2;
      v2b_2 = (((uint2((uint(i)).xx) == uint2(0u, 1u))) ? ((true).xx) : (v_7));
    }
  }
}

