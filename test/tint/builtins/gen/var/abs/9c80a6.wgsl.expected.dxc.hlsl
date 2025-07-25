//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 abs_9c80a6() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  int4 res = max(v, -(v));
  return res;
}

void fragment_main() {
  prevent_dce.Store4(0u, asuint(abs_9c80a6()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int4 abs_9c80a6() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  int4 res = max(v, -(v));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store4(0u, asuint(abs_9c80a6()));
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


int4 abs_9c80a6() {
  int4 arg_0 = (int(1)).xxxx;
  int4 v = arg_0;
  int4 res = max(v, -(v));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = abs_9c80a6();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

