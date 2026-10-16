//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_77f9cc() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  uint arg_3 = 1u;
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(v.x, v.y, v.z, v.w);
  uint v_1 = min(arg_3, (v.w - 1u));
  int3 v_2 = int3(arg_1, arg_2);
  int4 res = arg_0.Load(v_2, int(v_1));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_77f9cc()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_77f9cc() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  uint arg_3 = 1u;
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(v.x, v.y, v.z, v.w);
  uint v_1 = min(arg_3, (v.w - 1u));
  int3 v_2 = int3(arg_1, arg_2);
  int4 res = arg_0.Load(v_2, int(v_1));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_77f9cc()));
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


Texture2DMSArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_77f9cc() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  uint arg_3 = 1u;
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(v.x, v.y, v.z, v.w);
  uint v_1 = min(arg_3, (v.w - 1u));
  int3 v_2 = int3(arg_1, arg_2);
  int4 res = arg_0.Load(v_2, int(v_1));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_3 = (VertexOutput)0;
  v_3.pos = (0.0f).xxxx;
  v_3.prevent_dce = textureLoad_77f9cc();
  VertexOutput v_4 = v_3;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

