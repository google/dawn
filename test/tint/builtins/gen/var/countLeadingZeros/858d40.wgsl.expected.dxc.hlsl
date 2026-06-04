//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 countLeadingZeros_858d40() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = select((v <= (65535u).xx), (16u).xx, (0u).xx);
  uint2 v_2 = (v << v_1);
  uint2 v_3 = select((v_2 <= (16777215u).xx), (8u).xx, (0u).xx);
  uint2 v_4 = (v_2 << v_3);
  uint2 v_5 = select((v_4 <= (268435455u).xx), (4u).xx, (0u).xx);
  uint2 v_6 = (v_4 << v_5);
  uint2 v_7 = select((v_6 <= (1073741823u).xx), (2u).xx, (0u).xx);
  uint2 v_8 = (v_6 << v_7);
  uint2 v_9 = select((v_8 == (0u).xx), (1u).xx, (0u).xx);
  int2 res = asint(((v_1 | (v_3 | (v_5 | (v_7 | (select((v_8 <= (2147483647u).xx), (1u).xx, (0u).xx) | v_9))))) + v_9));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(countLeadingZeros_858d40()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 countLeadingZeros_858d40() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = select((v <= (65535u).xx), (16u).xx, (0u).xx);
  uint2 v_2 = (v << v_1);
  uint2 v_3 = select((v_2 <= (16777215u).xx), (8u).xx, (0u).xx);
  uint2 v_4 = (v_2 << v_3);
  uint2 v_5 = select((v_4 <= (268435455u).xx), (4u).xx, (0u).xx);
  uint2 v_6 = (v_4 << v_5);
  uint2 v_7 = select((v_6 <= (1073741823u).xx), (2u).xx, (0u).xx);
  uint2 v_8 = (v_6 << v_7);
  uint2 v_9 = select((v_8 == (0u).xx), (1u).xx, (0u).xx);
  int2 res = asint(((v_1 | (v_3 | (v_5 | (v_7 | (select((v_8 <= (2147483647u).xx), (1u).xx, (0u).xx) | v_9))))) + v_9));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(countLeadingZeros_858d40()));
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


int2 countLeadingZeros_858d40() {
  int2 arg_0 = (int(1)).xx;
  uint2 v = asuint(arg_0);
  uint2 v_1 = select((v <= (65535u).xx), (16u).xx, (0u).xx);
  uint2 v_2 = (v << v_1);
  uint2 v_3 = select((v_2 <= (16777215u).xx), (8u).xx, (0u).xx);
  uint2 v_4 = (v_2 << v_3);
  uint2 v_5 = select((v_4 <= (268435455u).xx), (4u).xx, (0u).xx);
  uint2 v_6 = (v_4 << v_5);
  uint2 v_7 = select((v_6 <= (1073741823u).xx), (2u).xx, (0u).xx);
  uint2 v_8 = (v_6 << v_7);
  uint2 v_9 = select((v_8 == (0u).xx), (1u).xx, (0u).xx);
  int2 res = asint(((v_1 | (v_3 | (v_5 | (v_7 | (select((v_8 <= (2147483647u).xx), (1u).xx, (0u).xx) | v_9))))) + v_9));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_10 = (VertexOutput)0;
  v_10.pos = (0.0f).xxxx;
  v_10.prevent_dce = countLeadingZeros_858d40();
  VertexOutput v_11 = v_10;
  return v_11;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_12 = vertex_main_inner();
  vertex_main_outputs v_13 = {v_12.prevent_dce, v_12.pos};
  return v_13;
}

