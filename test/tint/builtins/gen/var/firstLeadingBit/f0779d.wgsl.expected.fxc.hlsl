//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = ((((v & 4294901760u) == 0u)) ? (0u) : (16u));
  uint v_2 = (v >> v_1);
  uint v_3 = ((((v_2 & 65280u) == 0u)) ? (0u) : (8u));
  uint v_4 = (v_2 >> v_3);
  uint v_5 = ((((v_4 & 240u) == 0u)) ? (0u) : (4u));
  uint v_6 = (v_4 >> v_5);
  uint v_7 = ((((v_6 & 12u) == 0u)) ? (0u) : (2u));
  uint v_8 = (v_6 >> v_7);
  uint res = (((v_8 == 0u)) ? (4294967295u) : ((v_1 | (v_3 | (v_5 | (v_7 | ((((v_8 & 2u) == 0u)) ? (0u) : (1u))))))));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, firstLeadingBit_f0779d());
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = ((((v & 4294901760u) == 0u)) ? (0u) : (16u));
  uint v_2 = (v >> v_1);
  uint v_3 = ((((v_2 & 65280u) == 0u)) ? (0u) : (8u));
  uint v_4 = (v_2 >> v_3);
  uint v_5 = ((((v_4 & 240u) == 0u)) ? (0u) : (4u));
  uint v_6 = (v_4 >> v_5);
  uint v_7 = ((((v_6 & 12u) == 0u)) ? (0u) : (2u));
  uint v_8 = (v_6 >> v_7);
  uint res = (((v_8 == 0u)) ? (4294967295u) : ((v_1 | (v_3 | (v_5 | (v_7 | ((((v_8 & 2u) == 0u)) ? (0u) : (1u))))))));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, firstLeadingBit_f0779d());
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  uint prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation uint VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


uint firstLeadingBit_f0779d() {
  uint arg_0 = 1u;
  uint v = arg_0;
  uint v_1 = ((((v & 4294901760u) == 0u)) ? (0u) : (16u));
  uint v_2 = (v >> v_1);
  uint v_3 = ((((v_2 & 65280u) == 0u)) ? (0u) : (8u));
  uint v_4 = (v_2 >> v_3);
  uint v_5 = ((((v_4 & 240u) == 0u)) ? (0u) : (4u));
  uint v_6 = (v_4 >> v_5);
  uint v_7 = ((((v_6 & 12u) == 0u)) ? (0u) : (2u));
  uint v_8 = (v_6 >> v_7);
  uint res = (((v_8 == 0u)) ? (4294967295u) : ((v_1 | (v_3 | (v_5 | (v_7 | ((((v_8 & 2u) == 0u)) ? (0u) : (1u))))))));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_9 = (VertexOutput)0;
  v_9.pos = (0.0f).xxxx;
  v_9.prevent_dce = firstLeadingBit_f0779d();
  VertexOutput v_10 = v_9;
  return v_10;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_11 = vertex_main_inner();
  vertex_main_outputs v_12 = {v_11.prevent_dce, v_11.pos};
  return v_12;
}

