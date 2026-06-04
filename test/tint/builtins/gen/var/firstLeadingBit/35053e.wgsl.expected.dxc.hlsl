//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 firstLeadingBit_35053e() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = select((v < (2147483648u).xxx), v, ~(v));
  uint3 v_2 = select(((v_1 & (4294901760u).xxx) == (0u).xxx), (0u).xxx, (16u).xxx);
  uint3 v_3 = (v_1 >> v_2);
  uint3 v_4 = select(((v_3 & (65280u).xxx) == (0u).xxx), (0u).xxx, (8u).xxx);
  uint3 v_5 = (v_3 >> v_4);
  uint3 v_6 = select(((v_5 & (240u).xxx) == (0u).xxx), (0u).xxx, (4u).xxx);
  uint3 v_7 = (v_5 >> v_6);
  uint3 v_8 = select(((v_7 & (12u).xxx) == (0u).xxx), (0u).xxx, (2u).xxx);
  uint3 v_9 = (v_7 >> v_8);
  int3 res = asint(select((v_9 == (0u).xxx), (4294967295u).xxx, (v_2 | (v_4 | (v_6 | (v_8 | select(((v_9 & (2u).xxx) == (0u).xxx), (0u).xxx, (1u).xxx)))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(firstLeadingBit_35053e()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 firstLeadingBit_35053e() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = select((v < (2147483648u).xxx), v, ~(v));
  uint3 v_2 = select(((v_1 & (4294901760u).xxx) == (0u).xxx), (0u).xxx, (16u).xxx);
  uint3 v_3 = (v_1 >> v_2);
  uint3 v_4 = select(((v_3 & (65280u).xxx) == (0u).xxx), (0u).xxx, (8u).xxx);
  uint3 v_5 = (v_3 >> v_4);
  uint3 v_6 = select(((v_5 & (240u).xxx) == (0u).xxx), (0u).xxx, (4u).xxx);
  uint3 v_7 = (v_5 >> v_6);
  uint3 v_8 = select(((v_7 & (12u).xxx) == (0u).xxx), (0u).xxx, (2u).xxx);
  uint3 v_9 = (v_7 >> v_8);
  int3 res = asint(select((v_9 == (0u).xxx), (4294967295u).xxx, (v_2 | (v_4 | (v_6 | (v_8 | select(((v_9 & (2u).xxx) == (0u).xxx), (0u).xxx, (1u).xxx)))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(firstLeadingBit_35053e()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int3 firstLeadingBit_35053e() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = select((v < (2147483648u).xxx), v, ~(v));
  uint3 v_2 = select(((v_1 & (4294901760u).xxx) == (0u).xxx), (0u).xxx, (16u).xxx);
  uint3 v_3 = (v_1 >> v_2);
  uint3 v_4 = select(((v_3 & (65280u).xxx) == (0u).xxx), (0u).xxx, (8u).xxx);
  uint3 v_5 = (v_3 >> v_4);
  uint3 v_6 = select(((v_5 & (240u).xxx) == (0u).xxx), (0u).xxx, (4u).xxx);
  uint3 v_7 = (v_5 >> v_6);
  uint3 v_8 = select(((v_7 & (12u).xxx) == (0u).xxx), (0u).xxx, (2u).xxx);
  uint3 v_9 = (v_7 >> v_8);
  int3 res = asint(select((v_9 == (0u).xxx), (4294967295u).xxx, (v_2 | (v_4 | (v_6 | (v_8 | select(((v_9 & (2u).xxx) == (0u).xxx), (0u).xxx, (1u).xxx)))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_10 = (VertexOutput)0;
  v_10.pos = (0.0f).xxxx;
  v_10.prevent_dce = firstLeadingBit_35053e();
  VertexOutput v_11 = v_10;
  return v_11;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_12 = vertex_main_inner();
  vertex_main_outputs v_13 = {v_12.prevent_dce, v_12.pos};
  return v_13;
}

