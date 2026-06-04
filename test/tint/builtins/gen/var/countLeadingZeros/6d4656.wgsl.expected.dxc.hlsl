//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int countLeadingZeros_6d4656() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = select((v <= 65535u), 16u, 0u);
  uint v_2 = (v << v_1);
  uint v_3 = select((v_2 <= 16777215u), 8u, 0u);
  uint v_4 = (v_2 << v_3);
  uint v_5 = select((v_4 <= 268435455u), 4u, 0u);
  uint v_6 = (v_4 << v_5);
  uint v_7 = select((v_6 <= 1073741823u), 2u, 0u);
  uint v_8 = (v_6 << v_7);
  uint v_9 = select((v_8 == 0u), 1u, 0u);
  int res = asint(((v_1 | (v_3 | (v_5 | (v_7 | (select((v_8 <= 2147483647u), 1u, 0u) | v_9))))) + v_9));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(countLeadingZeros_6d4656()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int countLeadingZeros_6d4656() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = select((v <= 65535u), 16u, 0u);
  uint v_2 = (v << v_1);
  uint v_3 = select((v_2 <= 16777215u), 8u, 0u);
  uint v_4 = (v_2 << v_3);
  uint v_5 = select((v_4 <= 268435455u), 4u, 0u);
  uint v_6 = (v_4 << v_5);
  uint v_7 = select((v_6 <= 1073741823u), 2u, 0u);
  uint v_8 = (v_6 << v_7);
  uint v_9 = select((v_8 == 0u), 1u, 0u);
  int res = asint(((v_1 | (v_3 | (v_5 | (v_7 | (select((v_8 <= 2147483647u), 1u, 0u) | v_9))))) + v_9));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(countLeadingZeros_6d4656()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  int prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


int countLeadingZeros_6d4656() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = select((v <= 65535u), 16u, 0u);
  uint v_2 = (v << v_1);
  uint v_3 = select((v_2 <= 16777215u), 8u, 0u);
  uint v_4 = (v_2 << v_3);
  uint v_5 = select((v_4 <= 268435455u), 4u, 0u);
  uint v_6 = (v_4 << v_5);
  uint v_7 = select((v_6 <= 1073741823u), 2u, 0u);
  uint v_8 = (v_6 << v_7);
  uint v_9 = select((v_8 == 0u), 1u, 0u);
  int res = asint(((v_1 | (v_3 | (v_5 | (v_7 | (select((v_8 <= 2147483647u), 1u, 0u) | v_9))))) + v_9));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_10 = (VertexOutput)0;
  v_10.pos = (0.0f).xxxx;
  v_10.prevent_dce = countLeadingZeros_6d4656();
  VertexOutput v_11 = v_10;
  return v_11;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_12 = vertex_main_inner();
  vertex_main_outputs v_13 = {v_12.prevent_dce, v_12.pos};
  return v_13;
}

