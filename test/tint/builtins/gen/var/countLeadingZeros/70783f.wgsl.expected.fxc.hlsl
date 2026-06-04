//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 countLeadingZeros_70783f() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 v_1 = (((v <= (65535u).xx)) ? ((16u).xx) : ((0u).xx));
  uint2 v_2 = (v << v_1);
  uint2 v_3 = (((v_2 <= (16777215u).xx)) ? ((8u).xx) : ((0u).xx));
  uint2 v_4 = (v_2 << v_3);
  uint2 v_5 = (((v_4 <= (268435455u).xx)) ? ((4u).xx) : ((0u).xx));
  uint2 v_6 = (v_4 << v_5);
  uint2 v_7 = (((v_6 <= (1073741823u).xx)) ? ((2u).xx) : ((0u).xx));
  uint2 v_8 = (v_6 << v_7);
  uint2 v_9 = (((v_8 <= (2147483647u).xx)) ? ((1u).xx) : ((0u).xx));
  uint2 v_10 = (((v_8 == (0u).xx)) ? ((1u).xx) : ((0u).xx));
  uint2 res = ((v_1 | (v_3 | (v_5 | (v_7 | (v_9 | v_10))))) + v_10);
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, countLeadingZeros_70783f());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 countLeadingZeros_70783f() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 v_1 = (((v <= (65535u).xx)) ? ((16u).xx) : ((0u).xx));
  uint2 v_2 = (v << v_1);
  uint2 v_3 = (((v_2 <= (16777215u).xx)) ? ((8u).xx) : ((0u).xx));
  uint2 v_4 = (v_2 << v_3);
  uint2 v_5 = (((v_4 <= (268435455u).xx)) ? ((4u).xx) : ((0u).xx));
  uint2 v_6 = (v_4 << v_5);
  uint2 v_7 = (((v_6 <= (1073741823u).xx)) ? ((2u).xx) : ((0u).xx));
  uint2 v_8 = (v_6 << v_7);
  uint2 v_9 = (((v_8 <= (2147483647u).xx)) ? ((1u).xx) : ((0u).xx));
  uint2 v_10 = (((v_8 == (0u).xx)) ? ((1u).xx) : ((0u).xx));
  uint2 res = ((v_1 | (v_3 | (v_5 | (v_7 | (v_9 | v_10))))) + v_10);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, countLeadingZeros_70783f());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint2 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint2 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint2 countLeadingZeros_70783f() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 v_1 = (((v <= (65535u).xx)) ? ((16u).xx) : ((0u).xx));
  uint2 v_2 = (v << v_1);
  uint2 v_3 = (((v_2 <= (16777215u).xx)) ? ((8u).xx) : ((0u).xx));
  uint2 v_4 = (v_2 << v_3);
  uint2 v_5 = (((v_4 <= (268435455u).xx)) ? ((4u).xx) : ((0u).xx));
  uint2 v_6 = (v_4 << v_5);
  uint2 v_7 = (((v_6 <= (1073741823u).xx)) ? ((2u).xx) : ((0u).xx));
  uint2 v_8 = (v_6 << v_7);
  uint2 v_9 = (((v_8 <= (2147483647u).xx)) ? ((1u).xx) : ((0u).xx));
  uint2 v_10 = (((v_8 == (0u).xx)) ? ((1u).xx) : ((0u).xx));
  uint2 res = ((v_1 | (v_3 | (v_5 | (v_7 | (v_9 | v_10))))) + v_10);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_11 = (VertexOutput)0;
  v_11.pos = (0.0f).xxxx;
  v_11.prevent_dce = countLeadingZeros_70783f();
  VertexOutput v_12 = v_11;
  return v_12;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_13 = vertex_main_inner();
  vertex_main_outputs v_14 = {v_13.prevent_dce, v_13.pos};
  return v_14;
}

