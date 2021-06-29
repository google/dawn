void Set_float2(inout float2 vec, int idx, float val) {
  switch(idx) {
    case 0: vec[0] = val; break;
    case 1: vec[1] = val; break;
  }
}
void Set_int2(inout int2 vec, int idx, int val) {
  switch(idx) {
    case 0: vec[0] = val; break;
    case 1: vec[1] = val; break;
  }
}
void Set_uint2(inout uint2 vec, int idx, uint val) {
  switch(idx) {
    case 0: vec[0] = val; break;
    case 1: vec[1] = val; break;
  }
}
void Set_bool2(inout bool2 vec, int idx, bool val) {
  switch(idx) {
    case 0: vec[0] = val; break;
    case 1: vec[1] = val; break;
  }
}
[numthreads(1, 1, 1)]
void main() {
  float2 v2f = float2(0.0f, 0.0f);
  float3 v3f = float3(0.0f, 0.0f, 0.0f);
  float4 v4f = float4(0.0f, 0.0f, 0.0f, 0.0f);
  int2 v2i = int2(0, 0);
  int3 v3i = int3(0, 0, 0);
  int4 v4i = int4(0, 0, 0, 0);
  uint2 v2u = uint2(0u, 0u);
  uint3 v3u = uint3(0u, 0u, 0u);
  uint4 v4u = uint4(0u, 0u, 0u, 0u);
  bool2 v2b = bool2(false, false);
  bool3 v3b = bool3(false, false, false);
  bool4 v4b = bool4(false, false, false, false);
  {
    int i = 0;
    while (true) {
      if (!((i < 2))) {
        break;
      }
      Set_float2(v2f, i, 1.0f);
      Set_int2(v2i, i, 1);
      Set_uint2(v2u, i, 1u);
      Set_bool2(v2b, i, true);
      {
        i = (i + 1);
      }
    }
  }
  int i = 0;
  v3f[i] = 1.0f;
  v4f[i] = 1.0f;
  v3i[i] = 1;
  v4i[i] = 1;
  v3u[i] = 1u;
  v4u[i] = 1u;
  v3b[i] = true;
  v4b[i] = true;
  return;
}
