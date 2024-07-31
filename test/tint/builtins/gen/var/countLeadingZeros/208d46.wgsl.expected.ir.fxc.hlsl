struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
uint countLeadingZeros_208d46() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = (((v <= 65535u)) ? (16u) : (0u));
  uint v_2 = ((((v << v_1) <= 16777215u)) ? (8u) : (0u));
  uint v_3 = (((((v << v_1) << v_2) <= 268435455u)) ? (4u) : (0u));
  uint v_4 = ((((((v << v_1) << v_2) << v_3) <= 1073741823u)) ? (2u) : (0u));
  uint v_5 = (((((((v << v_1) << v_2) << v_3) << v_4) <= 2147483647u)) ? (1u) : (0u));
  uint v_6 = (((((((v << v_1) << v_2) << v_3) << v_4) == 0u)) ? (1u) : (0u));
  uint res = ((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, countLeadingZeros_208d46());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, countLeadingZeros_208d46());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = countLeadingZeros_208d46();
  VertexOutput v_7 = tint_symbol;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  VertexOutput v_9 = v_8;
  VertexOutput v_10 = v_8;
  vertex_main_outputs v_11 = {v_10.prevent_dce, v_9.pos};
  return v_11;
}

