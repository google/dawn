struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
int4 extractBits_fb850f() {
  int4 arg_0 = (int(1)).xxxx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  int4 v = arg_0;
  uint v_1 = arg_2;
  uint v_2 = min(arg_1, 32u);
  uint v_3 = (32u - min(32u, (v_2 + v_1)));
  int4 v_4 = (((v_3 < 32u)) ? ((v << uint4((v_3).xxxx))) : ((int(0)).xxxx));
  int4 res = ((((v_3 + v_2) < 32u)) ? ((v_4 >> uint4(((v_3 + v_2)).xxxx))) : (((v_4 >> (31u).xxxx) >> (1u).xxxx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(extractBits_fb850f()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(extractBits_fb850f()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = extractBits_fb850f();
  VertexOutput v_5 = tint_symbol;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

