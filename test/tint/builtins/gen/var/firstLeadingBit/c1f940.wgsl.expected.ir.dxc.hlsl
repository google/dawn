struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
int4 firstLeadingBit_c1f940() {
  int4 arg_0 = (int(1)).xxxx;
  uint4 v = asuint(arg_0);
  uint4 v_1 = (((v < (2147483648u).xxxx)) ? (v) : (~(v)));
  uint4 v_2 = ((((v_1 & (4294901760u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((16u).xxxx));
  uint4 v_3 = (((((v_1 >> v_2) & (65280u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((8u).xxxx));
  uint4 v_4 = ((((((v_1 >> v_2) >> v_3) & (240u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((4u).xxxx));
  uint4 v_5 = (((((((v_1 >> v_2) >> v_3) >> v_4) & (12u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((2u).xxxx));
  int4 res = asint((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) == (0u).xxxx)) ? ((4294967295u).xxxx) : ((v_2 | (v_3 | (v_4 | (v_5 | ((((((((v_1 >> v_2) >> v_3) >> v_4) >> v_5) & (2u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((1u).xxxx)))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(firstLeadingBit_c1f940()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(firstLeadingBit_c1f940()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = firstLeadingBit_c1f940();
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

