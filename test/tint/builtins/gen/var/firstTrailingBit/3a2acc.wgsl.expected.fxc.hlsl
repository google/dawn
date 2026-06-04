//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int firstTrailingBit_3a2acc() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = ((((v & 65535u) == 0u)) ? (16u) : (0u));
  uint v_2 = (v >> v_1);
  uint v_3 = ((((v_2 & 255u) == 0u)) ? (8u) : (0u));
  uint v_4 = (v_2 >> v_3);
  uint v_5 = ((((v_4 & 15u) == 0u)) ? (4u) : (0u));
  uint v_6 = (v_4 >> v_5);
  uint v_7 = ((((v_6 & 3u) == 0u)) ? (2u) : (0u));
  uint v_8 = (v_6 >> v_7);
  int res = asint((((v_8 == 0u)) ? (4294967295u) : ((v_1 | (v_3 | (v_5 | (v_7 | ((((v_8 & 1u) == 0u)) ? (1u) : (0u)))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(firstTrailingBit_3a2acc()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int firstTrailingBit_3a2acc() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = ((((v & 65535u) == 0u)) ? (16u) : (0u));
  uint v_2 = (v >> v_1);
  uint v_3 = ((((v_2 & 255u) == 0u)) ? (8u) : (0u));
  uint v_4 = (v_2 >> v_3);
  uint v_5 = ((((v_4 & 15u) == 0u)) ? (4u) : (0u));
  uint v_6 = (v_4 >> v_5);
  uint v_7 = ((((v_6 & 3u) == 0u)) ? (2u) : (0u));
  uint v_8 = (v_6 >> v_7);
  int res = asint((((v_8 == 0u)) ? (4294967295u) : ((v_1 | (v_3 | (v_5 | (v_7 | ((((v_8 & 1u) == 0u)) ? (1u) : (0u)))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(firstTrailingBit_3a2acc()));
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


int firstTrailingBit_3a2acc() {
  int arg_0 = int(1);
  uint v = asuint(arg_0);
  uint v_1 = ((((v & 65535u) == 0u)) ? (16u) : (0u));
  uint v_2 = (v >> v_1);
  uint v_3 = ((((v_2 & 255u) == 0u)) ? (8u) : (0u));
  uint v_4 = (v_2 >> v_3);
  uint v_5 = ((((v_4 & 15u) == 0u)) ? (4u) : (0u));
  uint v_6 = (v_4 >> v_5);
  uint v_7 = ((((v_6 & 3u) == 0u)) ? (2u) : (0u));
  uint v_8 = (v_6 >> v_7);
  int res = asint((((v_8 == 0u)) ? (4294967295u) : ((v_1 | (v_3 | (v_5 | (v_7 | ((((v_8 & 1u) == 0u)) ? (1u) : (0u)))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_9 = (VertexOutput)0;
  v_9.pos = (0.0f).xxxx;
  v_9.prevent_dce = firstTrailingBit_3a2acc();
  VertexOutput v_10 = v_9;
  return v_10;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_11 = vertex_main_inner();
  vertex_main_outputs v_12 = {v_11.prevent_dce, v_11.pos};
  return v_12;
}

