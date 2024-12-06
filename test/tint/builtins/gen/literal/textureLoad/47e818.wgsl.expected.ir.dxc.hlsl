//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<int4> arg_0 : register(t0, space1);
int4 textureLoad_47e818() {
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z, v.w);
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(1u, (v.w - 1u))), v_1.x, v_1.y, v_1.z, v_1.w);
  int3 v_2 = int3(min((1u).xxx, (v_1.xyz - (1u).xxx)));
  int4 res = int4(arg_0.Load(int4(v_2, int(min(1u, (v.w - 1u))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_47e818()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<int4> arg_0 : register(t0, space1);
int4 textureLoad_47e818() {
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z, v.w);
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(1u, (v.w - 1u))), v_1.x, v_1.y, v_1.z, v_1.w);
  int3 v_2 = int3(min((1u).xxx, (v_1.xyz - (1u).xxx)));
  int4 res = int4(arg_0.Load(int4(v_2, int(min(1u, (v.w - 1u))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_47e818()));
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


Texture3D<int4> arg_0 : register(t0, space1);
int4 textureLoad_47e818() {
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z, v.w);
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(1u, (v.w - 1u))), v_1.x, v_1.y, v_1.z, v_1.w);
  int3 v_2 = int3(min((1u).xxx, (v_1.xyz - (1u).xxx)));
  int4 res = int4(arg_0.Load(int4(v_2, int(min(1u, (v.w - 1u))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_47e818();
  VertexOutput v_3 = tint_symbol;
  return v_3;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_4 = vertex_main_inner();
  vertex_main_outputs v_5 = {v_4.prevent_dce, v_4.pos};
  return v_5;
}

