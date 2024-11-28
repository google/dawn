struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
Texture3D<int4> arg_0 : register(t0, space1);
int4 textureLoad_e35f72() {
  uint4 v = (0u).xxxx;
  arg_0.GetDimensions(0u, v.x, v.y, v.z, v.w);
  uint4 v_1 = (0u).xxxx;
  arg_0.GetDimensions(uint(min(1u, (v.w - 1u))), v_1.x, v_1.y, v_1.z, v_1.w);
  uint3 v_2 = (v_1.xyz - (1u).xxx);
  int3 v_3 = int3(min(uint3((int(1)).xxx), v_2));
  int4 res = int4(arg_0.Load(int4(v_3, int(min(1u, (v.w - 1u))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_e35f72()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(textureLoad_e35f72()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_e35f72();
  VertexOutput v_4 = tint_symbol;
  return v_4;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_5 = vertex_main_inner();
  vertex_main_outputs v_6 = {v_5.prevent_dce, v_5.pos};
  return v_6;
}

