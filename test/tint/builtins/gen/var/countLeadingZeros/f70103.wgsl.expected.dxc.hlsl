//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 countLeadingZeros_f70103() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = select((v <= (65535u).xxxx), (16u).xxxx, (0u).xxxx);
  uint4 v_2 = (v << v_1);
  uint4 v_3 = select((v_2 <= (16777215u).xxxx), (8u).xxxx, (0u).xxxx);
  uint4 v_4 = (v_2 << v_3);
  uint4 v_5 = select((v_4 <= (268435455u).xxxx), (4u).xxxx, (0u).xxxx);
  uint4 v_6 = (v_4 << v_5);
  uint4 v_7 = select((v_6 <= (1073741823u).xxxx), (2u).xxxx, (0u).xxxx);
  uint4 v_8 = (v_6 << v_7);
  uint4 v_9 = select((v_8 == (0u).xxxx), (1u).xxxx, (0u).xxxx);
  uint4 res = ((v_1 | (v_3 | (v_5 | (v_7 | (select((v_8 <= (2147483647u).xxxx), (1u).xxxx, (0u).xxxx) | v_9))))) + v_9);
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, countLeadingZeros_f70103());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 countLeadingZeros_f70103() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = select((v <= (65535u).xxxx), (16u).xxxx, (0u).xxxx);
  uint4 v_2 = (v << v_1);
  uint4 v_3 = select((v_2 <= (16777215u).xxxx), (8u).xxxx, (0u).xxxx);
  uint4 v_4 = (v_2 << v_3);
  uint4 v_5 = select((v_4 <= (268435455u).xxxx), (4u).xxxx, (0u).xxxx);
  uint4 v_6 = (v_4 << v_5);
  uint4 v_7 = select((v_6 <= (1073741823u).xxxx), (2u).xxxx, (0u).xxxx);
  uint4 v_8 = (v_6 << v_7);
  uint4 v_9 = select((v_8 == (0u).xxxx), (1u).xxxx, (0u).xxxx);
  uint4 res = ((v_1 | (v_3 | (v_5 | (v_7 | (select((v_8 <= (2147483647u).xxxx), (1u).xxxx, (0u).xxxx) | v_9))))) + v_9);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, countLeadingZeros_f70103());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint4 countLeadingZeros_f70103() {
  uint4 arg_0 = (1u).xxxx;
  uint4 v = arg_0;
  uint4 v_1 = select((v <= (65535u).xxxx), (16u).xxxx, (0u).xxxx);
  uint4 v_2 = (v << v_1);
  uint4 v_3 = select((v_2 <= (16777215u).xxxx), (8u).xxxx, (0u).xxxx);
  uint4 v_4 = (v_2 << v_3);
  uint4 v_5 = select((v_4 <= (268435455u).xxxx), (4u).xxxx, (0u).xxxx);
  uint4 v_6 = (v_4 << v_5);
  uint4 v_7 = select((v_6 <= (1073741823u).xxxx), (2u).xxxx, (0u).xxxx);
  uint4 v_8 = (v_6 << v_7);
  uint4 v_9 = select((v_8 == (0u).xxxx), (1u).xxxx, (0u).xxxx);
  uint4 res = ((v_1 | (v_3 | (v_5 | (v_7 | (select((v_8 <= (2147483647u).xxxx), (1u).xxxx, (0u).xxxx) | v_9))))) + v_9);
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_10 = (VertexOutput)0;
  v_10.pos = (0.0f).xxxx;
  v_10.prevent_dce = countLeadingZeros_f70103();
  VertexOutput v_11 = v_10;
  return v_11;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_12 = vertex_main_inner();
  vertex_main_outputs v_13 = {v_12.prevent_dce, v_12.pos};
  return v_13;
}

