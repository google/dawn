struct VertexOutput {
  float4 pos;
  uint2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint2 extractBits_f28f69() {
  uint2 arg_0 = (1u).xx;
  uint arg_1 = 1u;
  uint arg_2 = 1u;
  uint2 v = arg_0;
  uint v_1 = arg_2;
  uint v_2 = min(arg_1, 32u);
  uint v_3 = (32u - min(32u, (v_2 + v_1)));
  uint2 v_4 = (((v_3 < 32u)) ? ((v << uint2((v_3).xx))) : ((0u).xx));
  uint2 res = ((((v_3 + v_2) < 32u)) ? ((v_4 >> uint2(((v_3 + v_2)).xx))) : (((v_4 >> (31u).xx) >> (1u).xx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, extractBits_f28f69());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, extractBits_f28f69());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = extractBits_f28f69();
  VertexOutput v_5 = tint_symbol;
  return v_5;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_6 = vertex_main_inner();
  vertex_main_outputs v_7 = {v_6.prevent_dce, v_6.pos};
  return v_7;
}

