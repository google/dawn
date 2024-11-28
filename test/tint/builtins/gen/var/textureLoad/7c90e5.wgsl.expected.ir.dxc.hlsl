struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray<uint4> arg_0 : register(t0, space1);
uint4 textureLoad_7c90e5() {
  int2 arg_1 = (int(1)).xx;
  int arg_2 = int(1);
  int arg_3 = int(1);
  int2 v = arg_1;
  int v_1 = arg_3;
  uint3 v_2 = (0u).xxx;
  arg_0.GetDimensions(v_2.x, v_2.y, v_2.z);
  uint v_3 = min(uint(arg_2), (v_2.z - 1u));
  uint4 v_4 = (0u).xxxx;
  arg_0.GetDimensions(0u, v_4.x, v_4.y, v_4.z, v_4.w);
  uint v_5 = min(uint(v_1), (v_4.w - 1u));
  uint4 v_6 = (0u).xxxx;
  arg_0.GetDimensions(uint(v_5), v_6.x, v_6.y, v_6.z, v_6.w);
  uint2 v_7 = (v_6.xy - (1u).xx);
  int2 v_8 = int2(min(uint2(v), v_7));
  int v_9 = int(v_3);
  uint4 res = uint4(arg_0.Load(int4(v_8, v_9, int(v_5))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, textureLoad_7c90e5());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, textureLoad_7c90e5());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = textureLoad_7c90e5();
  VertexOutput v_10 = tint_symbol;
  return v_10;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_11 = vertex_main_inner();
  vertex_main_outputs v_12 = {v_11.prevent_dce, v_11.pos};
  return v_12;
}

