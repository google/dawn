//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_656d76() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  uint arg_3 = 1u;
  int2 v = arg_1;
  uint v_1 = arg_3;
  uint3 v_2 = (0u).xxx;
  arg_0.GetDimensions(v_2.x, v_2.y, v_2.z);
  uint v_3 = min(uint(arg_2), (v_2.z - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_4.x, v_4.y, v_4.z, v_4.w);
  uint4 v_5 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(v_1, (v_4.w - 1u))), v_5.x, v_5.y, v_5.z, v_5.w);
  uint2 v_6 = (v_5.xy - (1u).xx);
  int2 v_7 = int2(min(uint2(v), v_6));
  int v_8 = int(v_3);
  uint4 res = uint4(arg_0.Load(int4(v_7, v_8, int(min(v_1, (v_4.w - 1u))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_656d76());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_656d76() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  uint arg_3 = 1u;
  int2 v = arg_1;
  uint v_1 = arg_3;
  uint3 v_2 = (0u).xxx;
  arg_0.GetDimensions(v_2.x, v_2.y, v_2.z);
  uint v_3 = min(uint(arg_2), (v_2.z - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_4.x, v_4.y, v_4.z, v_4.w);
  uint4 v_5 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(v_1, (v_4.w - 1u))), v_5.x, v_5.y, v_5.z, v_5.w);
  uint2 v_6 = (v_5.xy - (1u).xx);
  int2 v_7 = int2(min(uint2(v), v_6));
  int v_8 = int(v_3);
  uint4 res = uint4(arg_0.Load(int4(v_7, v_8, int(min(v_1, (v_4.w - 1u))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_656d76());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


Texture2DArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_656d76() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  uint arg_3 = 1u;
  int2 v = arg_1;
  uint v_1 = arg_3;
  uint3 v_2 = (0u).xxx;
  arg_0.GetDimensions(v_2.x, v_2.y, v_2.z);
  uint v_3 = min(uint(arg_2), (v_2.z - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_4.x, v_4.y, v_4.z, v_4.w);
  uint4 v_5 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(v_1, (v_4.w - 1u))), v_5.x, v_5.y, v_5.z, v_5.w);
  uint2 v_6 = (v_5.xy - (1u).xx);
  int2 v_7 = int2(min(uint2(v), v_6));
  int v_8 = int(v_3);
  uint4 res = uint4(arg_0.Load(int4(v_7, v_8, int(min(v_1, (v_4.w - 1u))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_9 = (VertexOutput)0;
  v_9.pos = (0.0f).xxxx;
  v_9.prevent_dce = textureLoad_656d76();
  VertexOutput v_10 = v_9;
  return v_10;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_11 = vertex_main_inner();
  vertex_main_outputs v_12 = {v_11.prevent_dce, v_11.pos};
  return v_12;
}

