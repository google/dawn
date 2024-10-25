struct VertexOutput {
  float4 pos;
  uint2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint2 firstLeadingBit_6fe804() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 v_1 = ((((v & (4294901760u).xx) == (0u).xx)) ? ((0u).xx) : ((16u).xx));
  uint2 v_2 = (((((v >> v_1) & (65280u).xx) == (0u).xx)) ? ((0u).xx) : ((8u).xx));
  uint2 v_3 = ((((((v >> v_1) >> v_2) & (240u).xx) == (0u).xx)) ? ((0u).xx) : ((4u).xx));
  uint2 v_4 = (((((((v >> v_1) >> v_2) >> v_3) & (12u).xx) == (0u).xx)) ? ((0u).xx) : ((2u).xx));
  uint2 res = (((((((v >> v_1) >> v_2) >> v_3) >> v_4) == (0u).xx)) ? ((4294967295u).xx) : ((v_1 | (v_2 | (v_3 | (v_4 | ((((((((v >> v_1) >> v_2) >> v_3) >> v_4) & (2u).xx) == (0u).xx)) ? ((0u).xx) : ((1u).xx))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, firstLeadingBit_6fe804());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, firstLeadingBit_6fe804());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = firstLeadingBit_6fe804();
  VertexOutput v_5 = tint_symbol;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

