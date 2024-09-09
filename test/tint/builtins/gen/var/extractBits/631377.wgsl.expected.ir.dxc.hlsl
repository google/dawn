struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint4 extractBits_631377() {
  uint4 arg_0 = (1u).xxxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint4 v = arg_0;
  uint v_1 = arg_2;
  uint v_2 = min(arg_1, 32u);
  uint v_3 = (32u - min(32u, (v_2 + v_1)));
  uint4 v_4 = (((v_3 < 32u)) ? ((v << uint4((v_3).xxxx))) : ((0u).xxxx));
  uint4 res = ((((v_3 + v_2) < 32u)) ? ((v_4 >> uint4(((v_3 + v_2)).xxxx))) : (((v_4 >> (31u).xxxx) >> (1u).xxxx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, extractBits_631377());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, extractBits_631377());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = extractBits_631377();
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

