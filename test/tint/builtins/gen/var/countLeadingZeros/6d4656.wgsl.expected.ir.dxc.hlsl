struct VertexOutput {
  float4 pos;
  int prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
int countLeadingZeros_6d4656() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = (((v <= 65535u)) ? (16u) : (0u));
  uint v_2 = ((((v << v_1) <= 16777215u)) ? (8u) : (0u));
  uint v_3 = (((((v << v_1) << v_2) <= 268435455u)) ? (4u) : (0u));
  uint v_4 = ((((((v << v_1) << v_2) << v_3) <= 1073741823u)) ? (2u) : (0u));
  uint v_5 = (((((((v << v_1) << v_2) << v_3) << v_4) <= 2147483647u)) ? (1u) : (0u));
  uint v_6 = (((((((v << v_1) << v_2) << v_3) << v_4) == 0u)) ? (1u) : (0u));
  int res = asint(((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(countLeadingZeros_6d4656()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(countLeadingZeros_6d4656()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = countLeadingZeros_6d4656();
  VertexOutput v_7 = tint_symbol;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_8.pos};
  return v_9;
}

