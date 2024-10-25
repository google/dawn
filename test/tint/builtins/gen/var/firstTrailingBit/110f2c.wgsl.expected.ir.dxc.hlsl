struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint4 firstTrailingBit_110f2c() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = ((((v & (65535u).xxxx) == (0u).xxxx)) ? ((16u).xxxx) : ((0u).xxxx));
  uint4 v_2 = (((((v >> v_1) & (255u).xxxx) == (0u).xxxx)) ? ((8u).xxxx) : ((0u).xxxx));
  uint4 v_3 = ((((((v >> v_1) >> v_2) & (15u).xxxx) == (0u).xxxx)) ? ((4u).xxxx) : ((0u).xxxx));
  uint4 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (3u).xxxx) == (0u).xxxx)) ? ((2u).xxxx) : ((0u).xxxx));
  uint4 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxxx)) ? ((4294967295u).xxxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (1u).xxxx) == (0u).xxxx)) ? ((1u).xxxx) : ((0u).xxxx))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, firstTrailingBit_110f2c());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, firstTrailingBit_110f2c());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = firstTrailingBit_110f2c();
  VertexOutput v_5 = tint_symbol;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

