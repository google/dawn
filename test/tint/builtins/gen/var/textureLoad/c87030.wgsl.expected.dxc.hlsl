//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_c87030() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  uint arg_3 = 1u;
  int v = arg_2;
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z, v_1.w);
  uint v_2 = min(arg_3, (v_1.w - 1u));
  int3 v_3 = int3(int2(arg_1), v);
  uint4 res = arg_0.Load(v_3, int(v_2));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_c87030());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_c87030() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  uint arg_3 = 1u;
  int v = arg_2;
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z, v_1.w);
  uint v_2 = min(arg_3, (v_1.w - 1u));
  int3 v_3 = int3(int2(arg_1), v);
  uint4 res = arg_0.Load(v_3, int(v_2));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_c87030());
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
uint4 textureLoad_c87030() {
  uint2 arg_1 = (1u).xx;
  int arg_2 = int(1);
  uint arg_3 = 1u;
  int v = arg_2;
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(v_1.x, v_1.y, v_1.z, v_1.w);
  uint v_2 = min(arg_3, (v_1.w - 1u));
  int3 v_3 = int3(int2(arg_1), v);
  uint4 res = arg_0.Load(v_3, int(v_2));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_4 = (VertexOutput)0;
  v_4.pos = (0.0f).xxxx;
  v_4.prevent_dce = textureLoad_c87030();
  VertexOutput v_5 = v_4;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

