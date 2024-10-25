struct VertexOutput {
  float4 pos;
  int3 prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation int3 VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
int3 clamp_5f0819() {
  int3 arg_0 = (int(1)).xxx;
  int3 arg_1 = (int(1)).xxx;
  int3 arg_2 = (int(1)).xxx;
  int3 v = arg_2;
  int3 res = min(max(arg_0, arg_1), v);
  return res;
}

void fragment_main() {
  prevent_dce.Store3(0u, asuint(clamp_5f0819()));
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store3(0u, asuint(clamp_5f0819()));
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = clamp_5f0819();
  VertexOutput v_1 = tint_symbol;
  return v_1;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.prevent_dce, v_2.pos};
  return v_3;
}

