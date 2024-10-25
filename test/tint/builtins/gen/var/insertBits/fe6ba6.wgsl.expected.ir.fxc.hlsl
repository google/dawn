struct VertexOutput {
  float4 pos;
  int2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
int2 insertBits_fe6ba6() {
  int2 arg_0 = (int(1)).xx;
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 v = arg_0;
  int2 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (v_2 + arg_3);
  uint v_4 = (((v_2 < 32u)) ? ((1u << v_2)) : (0u));
  uint v_5 = ((v_4 - 1u) ^ ((((v_3 < 32u)) ? ((1u << v_3)) : (0u)) - 1u));
  int2 v_6 = (((v_2 < 32u)) ? ((v_1 << uint2((v_2).xx))) : ((int(0)).xx));
  int2 v_7 = (v_6 & int2((v_5).xx));
  int2 res = (v_7 | (v & int2((~(v_5)).xx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(insertBits_fe6ba6()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(insertBits_fe6ba6()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = insertBits_fe6ba6();
  VertexOutput v_8 = tint_symbol;
  return v_8;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_9 = vertex_main_inner();
  vertex_main_outputs v_10 = {v_9.prevent_dce, v_9.pos};
  return v_10;
}

