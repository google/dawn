struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
int4 insertBits_d86978() {
  int4 arg_0 = (int(1)).xxxx;
  int4 arg_1 = (int(1)).xxxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int4 v = arg_0;
  int4 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  int4 v_6 = (((v_2 < 32u)) ? ((v_1 << uint4((v_2).xxxx))) : ((int(0)).xxxx));
  int4 v_7 = (v_6 & int4((v_5).xxxx));
  int4 res = (v_7 | (v & int4((~(v_5)).xxxx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(insertBits_d86978()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(insertBits_d86978()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = insertBits_d86978();
  VertexOutput v_8 = tint_symbol;
  return v_8;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_9 = vertex_main_inner();
  vertex_main_outputs v_10 = {v_9.prevent_dce, v_9.pos};
  return v_10;
}

