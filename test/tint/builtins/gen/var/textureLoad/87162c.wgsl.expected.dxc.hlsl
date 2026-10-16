//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_87162c() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint v = arg_2;
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z, v_1.w);
  uint v_2 = min(arg_3, (v_1.w - 1u));
  int2 v_3 = int2(arg_1);
  int3 v_4 = int3(v_3, int(v));
  uint4 res = arg_0.Load(v_4, int(v_2));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_87162c());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_87162c() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint v = arg_2;
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z, v_1.w);
  uint v_2 = min(arg_3, (v_1.w - 1u));
  int2 v_3 = int2(arg_1);
  int3 v_4 = int3(v_3, int(v));
  uint4 res = arg_0.Load(v_4, int(v_2));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_87162c());
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
uint4 textureLoad_87162c() {
  uint2 arg_1 = (1u).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint v = arg_2;
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z, v_1.w);
  uint v_2 = min(arg_3, (v_1.w - 1u));
  int2 v_3 = int2(arg_1);
  int3 v_4 = int3(v_3, int(v));
  uint4 res = arg_0.Load(v_4, int(v_2));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_5 = (VertexOutput)0;
  v_5.pos = (0.0f).xxxx;
  v_5.prevent_dce = textureLoad_87162c();
  VertexOutput v_6 = v_5;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  vertex_main_outputs v_8 = {v_7.prevent_dce, v_7.pos};
  return v_8;
}

