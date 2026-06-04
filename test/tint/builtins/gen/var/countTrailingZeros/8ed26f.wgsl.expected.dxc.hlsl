//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 countTrailingZeros_8ed26f() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 v_1 = select(((v & (65535u).xxx) == (0u).xxx), (16u).xxx, (0u).xxx);
  uint3 v_2 = (v >> v_1);
  uint3 v_3 = select(((v_2 & (255u).xxx) == (0u).xxx), (8u).xxx, (0u).xxx);
  uint3 v_4 = (v_2 >> v_3);
  uint3 v_5 = select(((v_4 & (15u).xxx) == (0u).xxx), (4u).xxx, (0u).xxx);
  uint3 v_6 = (v_4 >> v_5);
  uint3 v_7 = select(((v_6 & (3u).xxx) == (0u).xxx), (2u).xxx, (0u).xxx);
  uint3 v_8 = (v_6 >> v_7);
  uint3 res = ((v_1 | (v_3 | (v_5 | (v_7 | select(((v_8 & (1u).xxx) == (0u).xxx), (1u).xxx, (0u).xxx))))) + select((v_8 == (0u).xxx), (1u).xxx, (0u).xxx));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, countTrailingZeros_8ed26f());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint3 countTrailingZeros_8ed26f() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 v_1 = select(((v & (65535u).xxx) == (0u).xxx), (16u).xxx, (0u).xxx);
  uint3 v_2 = (v >> v_1);
  uint3 v_3 = select(((v_2 & (255u).xxx) == (0u).xxx), (8u).xxx, (0u).xxx);
  uint3 v_4 = (v_2 >> v_3);
  uint3 v_5 = select(((v_4 & (15u).xxx) == (0u).xxx), (4u).xxx, (0u).xxx);
  uint3 v_6 = (v_4 >> v_5);
  uint3 v_7 = select(((v_6 & (3u).xxx) == (0u).xxx), (2u).xxx, (0u).xxx);
  uint3 v_8 = (v_6 >> v_7);
  uint3 res = ((v_1 | (v_3 | (v_5 | (v_7 | select(((v_8 & (1u).xxx) == (0u).xxx), (1u).xxx, (0u).xxx))))) + select((v_8 == (0u).xxx), (1u).xxx, (0u).xxx));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, countTrailingZeros_8ed26f());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint3 countTrailingZeros_8ed26f() {
  uint3 arg_0 = (1u).xxx;
  uint3 v = arg_0;
  uint3 v_1 = select(((v & (65535u).xxx) == (0u).xxx), (16u).xxx, (0u).xxx);
  uint3 v_2 = (v >> v_1);
  uint3 v_3 = select(((v_2 & (255u).xxx) == (0u).xxx), (8u).xxx, (0u).xxx);
  uint3 v_4 = (v_2 >> v_3);
  uint3 v_5 = select(((v_4 & (15u).xxx) == (0u).xxx), (4u).xxx, (0u).xxx);
  uint3 v_6 = (v_4 >> v_5);
  uint3 v_7 = select(((v_6 & (3u).xxx) == (0u).xxx), (2u).xxx, (0u).xxx);
  uint3 v_8 = (v_6 >> v_7);
  uint3 res = ((v_1 | (v_3 | (v_5 | (v_7 | select(((v_8 & (1u).xxx) == (0u).xxx), (1u).xxx, (0u).xxx))))) + select((v_8 == (0u).xxx), (1u).xxx, (0u).xxx));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_9 = (VertexOutput)0;
  v_9.pos = (0.0f).xxxx;
  v_9.prevent_dce = countTrailingZeros_8ed26f();
  VertexOutput v_10 = v_9;
  return v_10;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_11 = vertex_main_inner();
  vertex_main_outputs v_12 = {v_11.prevent_dce, v_11.pos};
  return v_12;
}

