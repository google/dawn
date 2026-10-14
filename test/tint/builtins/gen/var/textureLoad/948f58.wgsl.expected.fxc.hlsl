//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_948f58() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 v = arg_1;
  uint v_1 = arg_3;
  int3 v_2 = int3(v, int(arg_2));
  uint4 res = arg_0.Load(v_2, int(v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_948f58());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_948f58() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 v = arg_1;
  uint v_1 = arg_3;
  int3 v_2 = int3(v, int(arg_2));
  uint4 res = arg_0.Load(v_2, int(v_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_948f58());
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
uint4 textureLoad_948f58() {
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 v = arg_1;
  uint v_1 = arg_3;
  int3 v_2 = int3(v, int(arg_2));
  uint4 res = arg_0.Load(v_2, int(v_1));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_3 = (VertexOutput)0;
  v_3.pos = (0.0f).xxxx;
  v_3.prevent_dce = textureLoad_948f58();
  VertexOutput v_4 = v_3;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

