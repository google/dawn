//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture2DMSArray<int4> arg_0 : register(t0, space1);
int4 textureLoad_77f9cc() {
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(v.x, v.y, v.z, v.w);
  int4 res = arg_0.Load((int(1)).xxx, int(min(1u, (v.w - 1u))));
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
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(v.x, v.y, v.z, v.w);
  int4 res = arg_0.Load((int(1)).xxx, int(min(1u, (v.w - 1u))));
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
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(v.x, v.y, v.z, v.w);
  int4 res = arg_0.Load((int(1)).xxx, int(min(1u, (v.w - 1u))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = textureLoad_77f9cc();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

