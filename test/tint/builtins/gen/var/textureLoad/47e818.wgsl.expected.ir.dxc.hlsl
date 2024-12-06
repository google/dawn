//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<int4> arg_0 : register(t0, space1);
int4 textureLoad_47e818() {
  uint3 arg_1 = (1u).xxx;
  uint arg_2 = 1u;
  uint3 v = arg_1;
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_1.x, v_1.y, v_1.z, v_1.w);
  uint v_2 = min(arg_2, (v_1.w - 1u));
  uint4 v_3 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_2), v_3.x, v_3.y, v_3.z, v_3.w);
  int3 v_4 = int3(min(v, (v_3.xyz - (1u).xxx)));
  int4 res = int4(arg_0.Load(int4(v_4, int(v_2))));
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
  uint3 arg_1 = (1u).xxx;
  uint arg_2 = 1u;
  uint3 v = arg_1;
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_1.x, v_1.y, v_1.z, v_1.w);
  uint v_2 = min(arg_2, (v_1.w - 1u));
  uint4 v_3 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_2), v_3.x, v_3.y, v_3.z, v_3.w);
  int3 v_4 = int3(min(v, (v_3.xyz - (1u).xxx)));
  int4 res = int4(arg_0.Load(int4(v_4, int(v_2))));
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
  uint3 arg_1 = (1u).xxx;
  uint arg_2 = 1u;
  uint3 v = arg_1;
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_1.x, v_1.y, v_1.z, v_1.w);
  uint v_2 = min(arg_2, (v_1.w - 1u));
  uint4 v_3 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_2), v_3.x, v_3.y, v_3.z, v_3.w);
  int3 v_4 = int3(min(v, (v_3.xyz - (1u).xxx)));
  int4 res = int4(arg_0.Load(int4(v_4, int(v_2))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_47e818();
  VertexOutput v_5 = tint_symbol;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

