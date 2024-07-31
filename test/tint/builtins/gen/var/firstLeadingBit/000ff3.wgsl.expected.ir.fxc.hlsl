struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint4 firstLeadingBit_000ff3() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = ((((v & (4294901760u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((16u).xxxx));
  uint4 v_2 = (((((v >> v_1) & (65280u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((8u).xxxx));
  uint4 v_3 = ((((((v >> v_1) >> v_2) & (240u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((4u).xxxx));
  uint4 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((2u).xxxx));
  uint4 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xxxx)) ? ((4294967295u).xxxx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((1u).xxxx))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, firstLeadingBit_000ff3());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, firstLeadingBit_000ff3());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = firstLeadingBit_000ff3();
  VertexOutput v_5 = tint_symbol;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  VertexOutput v_7 = v_6;
  VertexOutput v_8 = v_6;
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_7.pos};
  return v_9;
}

