//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float trunc_eb83df() {
  float arg_0 = 1.5f;
  float v = arg_0;
  float res = (((v < 0.0f)) ? (ceil(v)) : (floor(v)));
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(trunc_eb83df()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
float trunc_eb83df() {
  float arg_0 = 1.5f;
  float v = arg_0;
  float res = (((v < 0.0f)) ? (ceil(v)) : (floor(v)));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(trunc_eb83df()));
}

//
// vertex_main
//
struct VertexOutput {
  float4 pos;
  float prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


float trunc_eb83df() {
  float arg_0 = 1.5f;
  float v = arg_0;
  float res = (((v < 0.0f)) ? (ceil(v)) : (floor(v)));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = trunc_eb83df();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

