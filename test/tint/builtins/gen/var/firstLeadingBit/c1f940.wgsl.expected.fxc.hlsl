//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 firstLeadingBit_c1f940() {
  int4 arg_0 = (int(1)).xxxx;
  uint4 v = asuint(arg_0);
  uint4 v_1 = (((v < (2147483648u).xxxx)) ? (v) : (~(v)));
  uint4 v_2 = ((((v_1 & (4294901760u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((16u).xxxx));
  uint4 v_3 = (v_1 >> v_2);
  uint4 v_4 = ((((v_3 & (65280u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((8u).xxxx));
  uint4 v_5 = (v_3 >> v_4);
  uint4 v_6 = ((((v_5 & (240u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((4u).xxxx));
  uint4 v_7 = (v_5 >> v_6);
  uint4 v_8 = ((((v_7 & (12u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((2u).xxxx));
  uint4 v_9 = (v_7 >> v_8);
  int4 res = asint((((v_9 == (0u).xxxx)) ? ((4294967295u).xxxx) : ((v_2 | (v_4 | (v_6 | (v_8 | ((((v_9 & (2u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((1u).xxxx)))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(firstLeadingBit_c1f940()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 firstLeadingBit_c1f940() {
  int4 arg_0 = (int(1)).xxxx;
  uint4 v = asuint(arg_0);
  uint4 v_1 = (((v < (2147483648u).xxxx)) ? (v) : (~(v)));
  uint4 v_2 = ((((v_1 & (4294901760u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((16u).xxxx));
  uint4 v_3 = (v_1 >> v_2);
  uint4 v_4 = ((((v_3 & (65280u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((8u).xxxx));
  uint4 v_5 = (v_3 >> v_4);
  uint4 v_6 = ((((v_5 & (240u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((4u).xxxx));
  uint4 v_7 = (v_5 >> v_6);
  uint4 v_8 = ((((v_7 & (12u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((2u).xxxx));
  uint4 v_9 = (v_7 >> v_8);
  int4 res = asint((((v_9 == (0u).xxxx)) ? ((4294967295u).xxxx) : ((v_2 | (v_4 | (v_6 | (v_8 | ((((v_9 & (2u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((1u).xxxx)))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(firstLeadingBit_c1f940()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int4 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int4 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int4 firstLeadingBit_c1f940() {
  int4 arg_0 = (int(1)).xxxx;
  uint4 v = asuint(arg_0);
  uint4 v_1 = (((v < (2147483648u).xxxx)) ? (v) : (~(v)));
  uint4 v_2 = ((((v_1 & (4294901760u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((16u).xxxx));
  uint4 v_3 = (v_1 >> v_2);
  uint4 v_4 = ((((v_3 & (65280u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((8u).xxxx));
  uint4 v_5 = (v_3 >> v_4);
  uint4 v_6 = ((((v_5 & (240u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((4u).xxxx));
  uint4 v_7 = (v_5 >> v_6);
  uint4 v_8 = ((((v_7 & (12u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((2u).xxxx));
  uint4 v_9 = (v_7 >> v_8);
  int4 res = asint((((v_9 == (0u).xxxx)) ? ((4294967295u).xxxx) : ((v_2 | (v_4 | (v_6 | (v_8 | ((((v_9 & (2u).xxxx) == (0u).xxxx)) ? ((0u).xxxx) : ((1u).xxxx)))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_10 = (VertexOutput)0;
  v_10.pos = (0.0f).xxxx;
  v_10.prevent_dce = firstLeadingBit_c1f940();
  VertexOutput v_11 = v_10;
  return v_11;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_12 = vertex_main_inner();
  vertex_main_outputs v_13 = {v_12.prevent_dce, v_12.pos};
  return v_13;
}

