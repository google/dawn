//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 insertBits_51ede1() {
  uint4 arg_0 = (1u).xxxx;
  uint4 arg_1 = (1u).xxxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint4 v = arg_0;
  uint4 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (min(v_2, 32u) + min(arg_3, 32u));
  uint v_4 = ((select((v_2 < 32u), (1u << v_2), 0u) - 1u) ^ (select((v_3 < 32u), (1u << v_3), 0u) - 1u));
  uint4 v_5 = select((v_2 < 32u), (v_1 << uint4((v_2).xxxx)), (0u).xxxx);
  uint4 v_6 = (v_5 & uint4((v_4).xxxx));
  uint4 res = (v_6 | (v & uint4((~(v_4)).xxxx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, insertBits_51ede1());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint4 insertBits_51ede1() {
  uint4 arg_0 = (1u).xxxx;
  uint4 arg_1 = (1u).xxxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint4 v = arg_0;
  uint4 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (min(v_2, 32u) + min(arg_3, 32u));
  uint v_4 = ((select((v_2 < 32u), (1u << v_2), 0u) - 1u) ^ (select((v_3 < 32u), (1u << v_3), 0u) - 1u));
  uint4 v_5 = select((v_2 < 32u), (v_1 << uint4((v_2).xxxx)), (0u).xxxx);
  uint4 v_6 = (v_5 & uint4((v_4).xxxx));
  uint4 res = (v_6 | (v & uint4((~(v_4)).xxxx)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, insertBits_51ede1());
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


uint4 insertBits_51ede1() {
  uint4 arg_0 = (1u).xxxx;
  uint4 arg_1 = (1u).xxxx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  uint4 v = arg_0;
  uint4 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (min(v_2, 32u) + min(arg_3, 32u));
  uint v_4 = ((select((v_2 < 32u), (1u << v_2), 0u) - 1u) ^ (select((v_3 < 32u), (1u << v_3), 0u) - 1u));
  uint4 v_5 = select((v_2 < 32u), (v_1 << uint4((v_2).xxxx)), (0u).xxxx);
  uint4 v_6 = (v_5 & uint4((v_4).xxxx));
  uint4 res = (v_6 | (v & uint4((~(v_4)).xxxx)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_7 = (VertexOutput)0;
  v_7.pos = (0.0f).xxxx;
  v_7.prevent_dce = insertBits_51ede1();
  VertexOutput v_8 = v_7;
  return v_8;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_9 = vertex_main_inner();
  vertex_main_outputs v_10 = {v_9.prevent_dce, v_9.pos};
  return v_10;
}

