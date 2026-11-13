//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_9d104c() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  int arg_3 = int(1);
  uint2 v = arg_1;
  int v_1 = arg_2;
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(v_2.x, v_2.y, v_2.z, v_2.w);
  uint v_3 = (v_2.w - 1u);
  uint v_4 = min(uint(arg_3), v_3);
  int3 v_5 = int3(int2(v), v_1);
  uint4 res = arg_0.Load(v_5, int(v_4));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_9d104c());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_9d104c() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  int arg_3 = int(1);
  uint2 v = arg_1;
  int v_1 = arg_2;
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(v_2.x, v_2.y, v_2.z, v_2.w);
  uint v_3 = (v_2.w - 1u);
  uint v_4 = min(uint(arg_3), v_3);
  int3 v_5 = int3(int2(v), v_1);
  uint4 res = arg_0.Load(v_5, int(v_4));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_9d104c());
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


Texture2DMSArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_9d104c() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  int arg_3 = int(1);
  uint2 v = arg_1;
  int v_1 = arg_2;
  uint4 v_2 = (0u).xxxx;
  arg_0.GetDimensions(v_2.x, v_2.y, v_2.z, v_2.w);
  uint v_3 = (v_2.w - 1u);
  uint v_4 = min(uint(arg_3), v_3);
  int3 v_5 = int3(int2(v), v_1);
  uint4 res = arg_0.Load(v_5, int(v_4));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_6 = (VertexOutput)0;
  v_6.pos = (0.0f).xxxx;
  v_6.prevent_dce = textureLoad_9d104c();
  VertexOutput v_7 = v_6;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_8.pos};
  return v_9;
}

