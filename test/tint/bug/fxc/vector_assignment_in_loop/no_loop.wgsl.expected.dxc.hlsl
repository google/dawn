void set_float2(inout float2 vec, int idx, float val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_float4(inout float4 vec, int idx, float val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

void set_int2(inout int2 vec, int idx, int val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_int3(inout int3 vec, int idx, int val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_int4(inout int4 vec, int idx, int val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

void set_uint2(inout uint2 vec, int idx, uint val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_uint3(inout uint3 vec, int idx, uint val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_uint4(inout uint4 vec, int idx, uint val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
}

void set_bool2(inout bool2 vec, int idx, bool val) {
  vec = (idx.xx == int2(0, 1)) ? val.xx : vec;
}

void set_bool3(inout bool3 vec, int idx, bool val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void set_bool4(inout bool4 vec, int idx, bool val) {
  vec = (idx.xxxx == int4(0, 1, 2, 3)) ? val.xxxx : vec;
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
  int i = 0;
  set_float2(v2f, i, 1.0f);
  set_float3(v3f, i, 1.0f);
  set_float4(v4f, i, 1.0f);
  set_int2(v2i, i, 1);
  set_int3(v3i, i, 1);
  set_int4(v4i, i, 1);
  set_uint2(v2u, i, 1u);
  set_uint3(v3u, i, 1u);
  set_uint4(v4u, i, 1u);
  set_bool2(v2b, i, true);
  set_bool3(v3b, i, true);
  set_bool4(v4b, i, true);
  return;
}
