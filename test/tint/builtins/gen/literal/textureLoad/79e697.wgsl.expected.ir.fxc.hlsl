//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_79e697() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(uint(int(1)), (v.z - 1u));
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_2.x, v_2.y, v_2.z, v_2.w);
  uint v_3 = min(uint(int(1)), (v_2.w - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_3), v_4.x, v_4.y, v_4.z, v_4.w);
  uint2 v_5 = (v_4.xy - (1u).xx);
  int2 v_6 = int2(min(uint2((int(1)).xx), v_5));
  int v_7 = int(v_1);
  int4 res = int4(arg_0.Load(int4(v_6, v_7, int(v_3))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_79e697()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_79e697() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(uint(int(1)), (v.z - 1u));
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_2.x, v_2.y, v_2.z, v_2.w);
  uint v_3 = min(uint(int(1)), (v_2.w - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_3), v_4.x, v_4.y, v_4.z, v_4.w);
  uint2 v_5 = (v_4.xy - (1u).xx);
  int2 v_6 = int2(min(uint2((int(1)).xx), v_5));
  int v_7 = int(v_1);
  int4 res = int4(arg_0.Load(int4(v_6, v_7, int(v_3))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_79e697()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2DArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_79e697() {
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v.x, v.y, v.z);
  uint v_1 = min(uint(int(1)), (v.z - 1u));
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_2.x, v_2.y, v_2.z, v_2.w);
  uint v_3 = min(uint(int(1)), (v_2.w - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_3), v_4.x, v_4.y, v_4.z, v_4.w);
  uint2 v_5 = (v_4.xy - (1u).xx);
  int2 v_6 = int2(min(uint2((int(1)).xx), v_5));
  int v_7 = int(v_1);
  int4 res = int4(arg_0.Load(int4(v_6, v_7, int(v_3))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_8 = (VertexOutput)0;
  v_8.pos = (0.0f).xxxx;
  v_8.prevent_dce = textureLoad_79e697();
  VertexOutput v_9 = v_8;
  return v_9;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_10 = vertex_main_inner();
  vertex_main_outputs v_11 = {v_10.prevent_dce, v_10.pos};
  return v_11;
}

