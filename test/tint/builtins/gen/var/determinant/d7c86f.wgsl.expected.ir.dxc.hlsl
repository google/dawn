struct VertexOutput {
  float4 pos;
  float16_t prevent_dce;
};

struct vertex_main_outputs {
  nointerpolation float16_t VertexOutput_prevent_dce : TEXCOORD0;
  float4 VertexOutput_pos : SV_Position;
};


RWByteAddressBuffer prevent_dce : register(u0);
float16_t determinant_d7c86f() {
  matrix<float16_t, 3, 3> arg_0 = matrix<float16_t, 3, 3>((float16_t(1.0h)).xxx, (float16_t(1.0h)).xxx, (float16_t(1.0h)).xxx);
  float16_t res = determinant(arg_0);
  return res;
}

void fragment_main() {
  prevent_dce.Store<float16_t>(0u, determinant_d7c86f());
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store<float16_t>(0u, determinant_d7c86f());
}

VertexOutput vertex_main_inner() {
  VertexOutput tint_symbol = (VertexOutput)0;
  tint_symbol.pos = (0.0f).xxxx;
  tint_symbol.prevent_dce = determinant_d7c86f();
  VertexOutput v = tint_symbol;
  return v;
}

vertex_main_outputs vertex_main() {
  VertexOutput v_1 = vertex_main_inner();
  vertex_main_outputs v_2 = {v_1.prevent_dce, v_1.pos};
  return v_2;
}

