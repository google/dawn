//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 countTrailingZeros_1ad138() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 v_1 = select(((v & (65535u).xx) == (0u).xx), (16u).xx, (0u).xx);
  uint2 v_2 = (v >> v_1);
  uint2 v_3 = select(((v_2 & (255u).xx) == (0u).xx), (8u).xx, (0u).xx);
  uint2 v_4 = (v_2 >> v_3);
  uint2 v_5 = select(((v_4 & (15u).xx) == (0u).xx), (4u).xx, (0u).xx);
  uint2 v_6 = (v_4 >> v_5);
  uint2 v_7 = select(((v_6 & (3u).xx) == (0u).xx), (2u).xx, (0u).xx);
  uint2 v_8 = (v_6 >> v_7);
  uint2 res = ((v_1 | (v_3 | (v_5 | (v_7 | select(((v_8 & (1u).xx) == (0u).xx), (1u).xx, (0u).xx))))) + select((v_8 == (0u).xx), (1u).xx, (0u).xx));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, countTrailingZeros_1ad138());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint2 countTrailingZeros_1ad138() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 v_1 = select(((v & (65535u).xx) == (0u).xx), (16u).xx, (0u).xx);
  uint2 v_2 = (v >> v_1);
  uint2 v_3 = select(((v_2 & (255u).xx) == (0u).xx), (8u).xx, (0u).xx);
  uint2 v_4 = (v_2 >> v_3);
  uint2 v_5 = select(((v_4 & (15u).xx) == (0u).xx), (4u).xx, (0u).xx);
  uint2 v_6 = (v_4 >> v_5);
  uint2 v_7 = select(((v_6 & (3u).xx) == (0u).xx), (2u).xx, (0u).xx);
  uint2 v_8 = (v_6 >> v_7);
  uint2 res = ((v_1 | (v_3 | (v_5 | (v_7 | select(((v_8 & (1u).xx) == (0u).xx), (1u).xx, (0u).xx))))) + select((v_8 == (0u).xx), (1u).xx, (0u).xx));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, countTrailingZeros_1ad138());
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


uint2 countTrailingZeros_1ad138() {
  uint2 arg_0 = (1u).xx;
  uint2 v = arg_0;
  uint2 v_1 = select(((v & (65535u).xx) == (0u).xx), (16u).xx, (0u).xx);
  uint2 v_2 = (v >> v_1);
  uint2 v_3 = select(((v_2 & (255u).xx) == (0u).xx), (8u).xx, (0u).xx);
  uint2 v_4 = (v_2 >> v_3);
  uint2 v_5 = select(((v_4 & (15u).xx) == (0u).xx), (4u).xx, (0u).xx);
  uint2 v_6 = (v_4 >> v_5);
  uint2 v_7 = select(((v_6 & (3u).xx) == (0u).xx), (2u).xx, (0u).xx);
  uint2 v_8 = (v_6 >> v_7);
  uint2 res = ((v_1 | (v_3 | (v_5 | (v_7 | select(((v_8 & (1u).xx) == (0u).xx), (1u).xx, (0u).xx))))) + select((v_8 == (0u).xx), (1u).xx, (0u).xx));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_9 = (VertexOutput)0;
  v_9.pos = (0.0f).xxxx;
  v_9.prevent_dce = countTrailingZeros_1ad138();
  VertexOutput v_10 = v_9;
  return v_10;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_11 = vertex_main_inner();
  vertex_main_outputs v_12 = {v_11.prevent_dce, v_11.pos};
  return v_12;
}

