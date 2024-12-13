//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_8527b1() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint2 v = arg_1;
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint v_2 = min(arg_2, (v_1.z - 1u));
  uint4 v_3 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_3.x, v_3.y, v_3.z, v_3.w);
  uint v_4 = min(arg_3, (v_3.w - 1u));
  uint4 v_5 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_4), v_5.x, v_5.y, v_5.z, v_5.w);
  int2 v_6 = int2(min(v, (v_5.xy - (1u).xx)));
  int v_7 = int(v_2);
  uint4 res = uint4(arg_0.Load(int4(v_6, v_7, int(v_4))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_8527b1());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_8527b1() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint2 v = arg_1;
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint v_2 = min(arg_2, (v_1.z - 1u));
  uint4 v_3 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_3.x, v_3.y, v_3.z, v_3.w);
  uint v_4 = min(arg_3, (v_3.w - 1u));
  uint4 v_5 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_4), v_5.x, v_5.y, v_5.z, v_5.w);
  int2 v_6 = int2(min(v, (v_5.xy - (1u).xx)));
  int v_7 = int(v_2);
  uint4 res = uint4(arg_0.Load(int4(v_6, v_7, int(v_4))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_8527b1());
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
uint4 textureLoad_8527b1() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint2 v = arg_1;
  uint3 v_1 = (0u).xxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z);
  uint v_2 = min(arg_2, (v_1.z - 1u));
  uint4 v_3 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_3.x, v_3.y, v_3.z, v_3.w);
  uint v_4 = min(arg_3, (v_3.w - 1u));
  uint4 v_5 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_4), v_5.x, v_5.y, v_5.z, v_5.w);
  int2 v_6 = int2(min(v, (v_5.xy - (1u).xx)));
  int v_7 = int(v_2);
  uint4 res = uint4(arg_0.Load(int4(v_6, v_7, int(v_4))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_8 = (VertexOutput)0;
  v_8.pos = (0.0f).xxxx;
  v_8.prevent_dce = textureLoad_8527b1();
  VertexOutput v_9 = v_8;
  return v_9;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_10 = vertex_main_inner();
  vertex_main_outputs v_11 = {v_10.prevent_dce, v_10.pos};
  return v_11;
}

