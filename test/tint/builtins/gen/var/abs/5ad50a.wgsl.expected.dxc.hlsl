//
// fragment_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 abs_5ad50a() {
  int3 arg_0 = (int(1)).xxx;
  int3 v = arg_0;
  int3 res = max(v, -(v));
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(abs_5ad50a()));
}

//
// compute_main
//

RWByteAddressBuffer prevent_dce : register(u0);
int3 abs_5ad50a() {
  int3 arg_0 = (int(1)).xxx;
  int3 v = arg_0;
  int3 res = max(v, -(v));
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(abs_5ad50a()));
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


int3 abs_5ad50a() {
  int3 arg_0 = (int(1)).xxx;
  int3 v = arg_0;
  int3 res = max(v, -(v));
  return res;
}

VertexOutput vertex_main_inner() {
  VertexOutput v_1 = (VertexOutput)0;
  v_1.pos = (0.0f).xxxx;
  v_1.prevent_dce = abs_5ad50a();
  VertexOutput v_2 = v_1;
  return v_2;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.prevent_dce, v_3.pos};
  return v_4;
}

