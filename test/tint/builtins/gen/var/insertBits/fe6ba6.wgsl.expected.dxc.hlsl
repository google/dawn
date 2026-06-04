//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 insertBits_fe6ba6() {
  int2 arg_0 = (int(1)).xx;
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 v = arg_0;
  int2 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (min(v_2, 32u) + min(arg_3, 32u));
  uint v_4 = ((select((v_2 < 32u), (1u << v_2), 0u) - 1u) ^ (select((v_3 < 32u), (1u << v_3), 0u) - 1u));
  int2 v_5 = select((v_2 < 32u), asint((asuint(v_1) << uint2((v_2).xx))), (int(0)).xx);
  int2 v_6 = (v_5 & int2((int(v_4)).xx));
  int2 res = (v_6 | (v & int2((int(~(v_4))).xx)));
  return res;
}

void fragment_main() {
  prevent_dce.Store2(0u, asuint(insertBits_fe6ba6()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int2 insertBits_fe6ba6() {
  int2 arg_0 = (int(1)).xx;
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 v = arg_0;
  int2 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (min(v_2, 32u) + min(arg_3, 32u));
  uint v_4 = ((select((v_2 < 32u), (1u << v_2), 0u) - 1u) ^ (select((v_3 < 32u), (1u << v_3), 0u) - 1u));
  int2 v_5 = select((v_2 < 32u), asint((asuint(v_1) << uint2((v_2).xx))), (int(0)).xx);
  int2 v_6 = (v_5 & int2((int(v_4)).xx));
  int2 res = (v_6 | (v & int2((int(~(v_4))).xx)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store2(0u, asuint(insertBits_fe6ba6()));
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


int2 insertBits_fe6ba6() {
  int2 arg_0 = (int(1)).xx;
  int2 arg_1 = (int(1)).xx;
  uint arg_2 = 1u;
  uint arg_3 = 1u;
  int2 v = arg_0;
  int2 v_1 = arg_1;
  uint v_2 = arg_2;
  uint v_3 = (min(v_2, 32u) + min(arg_3, 32u));
  uint v_4 = ((select((v_2 < 32u), (1u << v_2), 0u) - 1u) ^ (select((v_3 < 32u), (1u << v_3), 0u) - 1u));
  int2 v_5 = select((v_2 < 32u), asint((asuint(v_1) << uint2((v_2).xx))), (int(0)).xx);
  int2 v_6 = (v_5 & int2((int(v_4)).xx));
  int2 res = (v_6 | (v & int2((int(~(v_4))).xx)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_7 = (VertexOutput)0;
  v_7.pos = (0.0f).xxxx;
  v_7.prevent_dce = insertBits_fe6ba6();
  VertexOutput v_8 = v_7;
  return v_8;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_9 = vertex_main_inner();
  vertex_main_outputs v_10 = {v_9.prevent_dce, v_9.pos};
  return v_10;
}

