struct VertexOutput {
  float4 pos;
  int2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
int2 countLeadingZeros_858d40() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = (((v <= (65535u).xx)) ? ((16u).xx) : ((0u).xx));
  uint2 v_2 = ((((v << v_1) <= (16777215u).xx)) ? ((8u).xx) : ((0u).xx));
  uint2 v_3 = (((((v << v_1) << v_2) <= (268435455u).xx)) ? ((4u).xx) : ((0u).xx));
  uint2 v_4 = ((((((v << v_1) << v_2) << v_3) <= (1073741823u).xx)) ? ((2u).xx) : ((0u).xx));
  uint2 v_5 = (((((((v << v_1) << v_2) << v_3) << v_4) <= (2147483647u).xx)) ? ((1u).xx) : ((0u).xx));
  uint2 v_6 = (((((((v << v_1) << v_2) << v_3) << v_4) == (0u).xx)) ? ((1u).xx) : ((0u).xx));
  int2 res = asint(((v_1 | (v_2 | (v_3 | (v_4 | (v_5 | v_6))))) + v_6));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(countLeadingZeros_858d40()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(countLeadingZeros_858d40()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = countLeadingZeros_858d40();
  VertexOutput v_7 = tint_symbol;
  return v_7;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_8 = vertex_main_inner();
  vertex_main_outputs v_9 = {v_8.prevent_dce, v_8.pos};
  return v_9;
}

