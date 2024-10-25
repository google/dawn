struct VertexOutput {
  float4 pos;
  uint3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint3 firstLeadingBit_3fd7d0() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 v_1 = ((((v & (4294901760u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((16u).xxx));
  uint3 v_2 = (((((v >> v_1) & (65280u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((8u).xxx));
  uint3 v_3 = ((((((v >> v_1) >> v_2) & (240u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((4u).xxx));
  uint3 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((2u).xxx));
  uint3 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxx)) ? ((4294967295u).xxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xxx) == (0u).xxx)) ? ((0u).xxx) : ((1u).xxx))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, firstLeadingBit_3fd7d0());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, firstLeadingBit_3fd7d0());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = firstLeadingBit_3fd7d0();
  VertexOutput v_5 = tint_symbol;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

