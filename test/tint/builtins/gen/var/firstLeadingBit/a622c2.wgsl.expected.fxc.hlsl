//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 firstLeadingBit_a622c2() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = (((v < (2147483648u).xx)) ? (v) : (~(v)));
  uint2 v_2 = ((((v_1 & (4294901760u).xx) == (0u).xx)) ? ((0u).xx) : ((16u).xx));
  uint2 v_3 = (v_1 >> v_2);
  uint2 v_4 = ((((v_3 & (65280u).xx) == (0u).xx)) ? ((0u).xx) : ((8u).xx));
  uint2 v_5 = (v_3 >> v_4);
  uint2 v_6 = ((((v_5 & (240u).xx) == (0u).xx)) ? ((0u).xx) : ((4u).xx));
  uint2 v_7 = (v_5 >> v_6);
  uint2 v_8 = ((((v_7 & (12u).xx) == (0u).xx)) ? ((0u).xx) : ((2u).xx));
  uint2 v_9 = (v_7 >> v_8);
  int2 res = asint((((v_9 == (0u).xx)) ? ((4294967295u).xx) : ((v_2 | (v_4 | (v_6 | (v_8 | ((((v_9 & (2u).xx) == (0u).xx)) ? ((0u).xx) : ((1u).xx)))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(firstLeadingBit_a622c2()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 firstLeadingBit_a622c2() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = (((v < (2147483648u).xx)) ? (v) : (~(v)));
  uint2 v_2 = ((((v_1 & (4294901760u).xx) == (0u).xx)) ? ((0u).xx) : ((16u).xx));
  uint2 v_3 = (v_1 >> v_2);
  uint2 v_4 = ((((v_3 & (65280u).xx) == (0u).xx)) ? ((0u).xx) : ((8u).xx));
  uint2 v_5 = (v_3 >> v_4);
  uint2 v_6 = ((((v_5 & (240u).xx) == (0u).xx)) ? ((0u).xx) : ((4u).xx));
  uint2 v_7 = (v_5 >> v_6);
  uint2 v_8 = ((((v_7 & (12u).xx) == (0u).xx)) ? ((0u).xx) : ((2u).xx));
  uint2 v_9 = (v_7 >> v_8);
  int2 res = asint((((v_9 == (0u).xx)) ? ((4294967295u).xx) : ((v_2 | (v_4 | (v_6 | (v_8 | ((((v_9 & (2u).xx) == (0u).xx)) ? ((0u).xx) : ((1u).xx)))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(firstLeadingBit_a622c2()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int2 firstLeadingBit_a622c2() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = (((v < (2147483648u).xx)) ? (v) : (~(v)));
  uint2 v_2 = ((((v_1 & (4294901760u).xx) == (0u).xx)) ? ((0u).xx) : ((16u).xx));
  uint2 v_3 = (v_1 >> v_2);
  uint2 v_4 = ((((v_3 & (65280u).xx) == (0u).xx)) ? ((0u).xx) : ((8u).xx));
  uint2 v_5 = (v_3 >> v_4);
  uint2 v_6 = ((((v_5 & (240u).xx) == (0u).xx)) ? ((0u).xx) : ((4u).xx));
  uint2 v_7 = (v_5 >> v_6);
  uint2 v_8 = ((((v_7 & (12u).xx) == (0u).xx)) ? ((0u).xx) : ((2u).xx));
  uint2 v_9 = (v_7 >> v_8);
  int2 res = asint((((v_9 == (0u).xx)) ? ((4294967295u).xx) : ((v_2 | (v_4 | (v_6 | (v_8 | ((((v_9 & (2u).xx) == (0u).xx)) ? ((0u).xx) : ((1u).xx)))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_10 = (VertexOutput)0;
  v_10.pos = (0.0f).xxxx;
  v_10.prevent_dce = firstLeadingBit_a622c2();
  VertexOutput v_11 = v_10;
  return v_11;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_12 = vertex_main_inner();
  vertex_main_outputs v_13 = {v_12.prevent_dce, v_12.pos};
  return v_13;
}

