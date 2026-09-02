
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
    for( ; (i < int(2)); i = asint((asuint(i) + 1u))) {
      float2 v = v2f;
      v2f = (((uint2((uint(i)).xx) == uint2(0u, 1u))) ? ((1.0f).xx) : (v));
      float3 v_1 = v3f;
      v3f = (((uint3((uint(i)).xxx) == uint3(0u, 1u, 2u))) ? ((1.0f).xxx) : (v_1));
      float4 v_2 = v4f;
      v4f = (((uint4((uint(i)).xxxx) == uint4(0u, 1u, 2u, 3u))) ? ((1.0f).xxxx) : (v_2));
      int2 v_3 = v2i;
      v2i = (((uint2((uint(i)).xx) == uint2(0u, 1u))) ? ((int(1)).xx) : (v_3));
      int3 v_4 = v3i;
      v3i = (((uint3((uint(i)).xxx) == uint3(0u, 1u, 2u))) ? ((int(1)).xxx) : (v_4));
      int4 v_5 = v4i;
      v4i = (((uint4((uint(i)).xxxx) == uint4(0u, 1u, 2u, 3u))) ? ((int(1)).xxxx) : (v_5));
      uint2 v_6 = v2u;
      v2u = (((uint2((uint(i)).xx) == uint2(0u, 1u))) ? ((1u).xx) : (v_6));
      uint3 v_7 = v3u;
      v3u = (((uint3((uint(i)).xxx) == uint3(0u, 1u, 2u))) ? ((1u).xxx) : (v_7));
      uint4 v_8 = v4u;
      v4u = (((uint4((uint(i)).xxxx) == uint4(0u, 1u, 2u, 3u))) ? ((1u).xxxx) : (v_8));
      bool2 v_9 = v2b;
      v2b = (((uint2((uint(i)).xx) == uint2(0u, 1u))) ? ((true).xx) : (v_9));
      bool3 v_10 = v3b;
      v3b = (((uint3((uint(i)).xxx) == uint3(0u, 1u, 2u))) ? ((true).xxx) : (v_10));
      bool4 v_11 = v4b;
      v4b = (((uint4((uint(i)).xxxx) == uint4(0u, 1u, 2u, 3u))) ? ((true).xxxx) : (v_11));
    }
  }
}

