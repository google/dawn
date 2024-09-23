struct VertexOutput {
  float4 pos;
  int prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
int firstLeadingBit_57a1a3() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = (((v < 2147483648u)) ? (v) : (~(v)));
  uint v_2 = ((((v_1 & 4294901760u) == 0u)) ? (0u) : (16u));
  uint v_3 = (((((v_1 >> v_2) & 65280u) == 0u)) ? (0u) : (8u));
  uint v_4 = ((((((v_1 >> v_2) >> v_3) & 240u) == 0u)) ? (0u) : (4u));
  uint v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & 12u) == 0u)) ? (0u) : (2u));
  int res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == 0u)) ? (4294967295u) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & 2u) == 0u)) ? (0u) : (1u)))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(firstLeadingBit_57a1a3()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(firstLeadingBit_57a1a3()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = firstLeadingBit_57a1a3();
  VertexOutput v_6 = tint_symbol;
  return v_6;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_7 = vertex_main_inner();
  VertexOutput v_8 = v_7;
  VertexOutput v_9 = v_7;
  vertex_main_outputs v_10 = {v_9.prevent_dce, v_8.pos};
  return v_10;
}

