//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 countLeadingZeros_7c38a6() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = (((v <= (65535u).xxx)) ? ((16u).xxx) : ((0u).xxx));
  uint3 v_2 = (v << v_1);
  uint3 v_3 = (((v_2 <= (16777215u).xxx)) ? ((8u).xxx) : ((0u).xxx));
  uint3 v_4 = (v_2 << v_3);
  uint3 v_5 = (((v_4 <= (268435455u).xxx)) ? ((4u).xxx) : ((0u).xxx));
  uint3 v_6 = (v_4 << v_5);
  uint3 v_7 = (((v_6 <= (1073741823u).xxx)) ? ((2u).xxx) : ((0u).xxx));
  uint3 v_8 = (v_6 << v_7);
  uint3 v_9 = (((v_8 <= (2147483647u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  uint3 v_10 = (((v_8 == (0u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  int3 res = asint(((v_1 | (v_3 | (v_5 | (v_7 | (v_9 | v_10))))) + v_10));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(countLeadingZeros_7c38a6()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 countLeadingZeros_7c38a6() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = (((v <= (65535u).xxx)) ? ((16u).xxx) : ((0u).xxx));
  uint3 v_2 = (v << v_1);
  uint3 v_3 = (((v_2 <= (16777215u).xxx)) ? ((8u).xxx) : ((0u).xxx));
  uint3 v_4 = (v_2 << v_3);
  uint3 v_5 = (((v_4 <= (268435455u).xxx)) ? ((4u).xxx) : ((0u).xxx));
  uint3 v_6 = (v_4 << v_5);
  uint3 v_7 = (((v_6 <= (1073741823u).xxx)) ? ((2u).xxx) : ((0u).xxx));
  uint3 v_8 = (v_6 << v_7);
  uint3 v_9 = (((v_8 <= (2147483647u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  uint3 v_10 = (((v_8 == (0u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  int3 res = asint(((v_1 | (v_3 | (v_5 | (v_7 | (v_9 | v_10))))) + v_10));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(countLeadingZeros_7c38a6()));
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


int3 countLeadingZeros_7c38a6() {
  int3 arg_0 = (int(1)).xxx;
  uint3 v = asuint(arg_0);
  uint3 v_1 = (((v <= (65535u).xxx)) ? ((16u).xxx) : ((0u).xxx));
  uint3 v_2 = (v << v_1);
  uint3 v_3 = (((v_2 <= (16777215u).xxx)) ? ((8u).xxx) : ((0u).xxx));
  uint3 v_4 = (v_2 << v_3);
  uint3 v_5 = (((v_4 <= (268435455u).xxx)) ? ((4u).xxx) : ((0u).xxx));
  uint3 v_6 = (v_4 << v_5);
  uint3 v_7 = (((v_6 <= (1073741823u).xxx)) ? ((2u).xxx) : ((0u).xxx));
  uint3 v_8 = (v_6 << v_7);
  uint3 v_9 = (((v_8 <= (2147483647u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  uint3 v_10 = (((v_8 == (0u).xxx)) ? ((1u).xxx) : ((0u).xxx));
  int3 res = asint(((v_1 | (v_3 | (v_5 | (v_7 | (v_9 | v_10))))) + v_10));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_11 = (VertexOutput)0;
  v_11.pos = (0.0f).xxxx;
  v_11.prevent_dce = countLeadingZeros_7c38a6();
  VertexOutput v_12 = v_11;
  return v_12;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_13 = vertex_main_inner();
  vertex_main_outputs v_14 = {v_13.prevent_dce, v_13.pos};
  return v_14;
}

