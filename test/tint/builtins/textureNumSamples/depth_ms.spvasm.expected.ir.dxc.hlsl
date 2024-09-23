struct vertex_main_out {
  float4 tint_symbol_1_1;
};

struct vertex_main_outputs {
  float4 vertex_main_out_tint_symbol_1_1 : SV_Position;
};


Texture2DMS<float4> arg_0 : register(t0, space1);
static float4 tint_symbol_1 = (0.0f).xxxx;
void textureNumSamples_a3c8a0() {
  int res = int(0);
  uint3 v = (0u).xxx;
  arg_0.GetDimensions(v[0u], v[1u], v[2u]);
  res = int(v.z);
}

void tint_symbol_2(float4 tint_symbol) {
  tint_symbol_1 = tint_symbol;
}

void vertex_main_1() {
  textureNumSamples_a3c8a0();
  tint_symbol_2((0.0f).xxxx);
}

vertex_main_out vertex_main_inner() {
  vertex_main_1();
  vertex_main_out v_1 = {tint_symbol_1};
  return v_1;
}

void fragment_main_1() {
  textureNumSamples_a3c8a0();
}

void fragment_main() {
  fragment_main_1();
}

void compute_main_1() {
  textureNumSamples_a3c8a0();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

vertex_main_outputs vertex_main() {
  vertex_main_out v_2 = vertex_main_inner();
  vertex_main_outputs v_3 = {v_2.tint_symbol_1_1};
  return v_3;
}

