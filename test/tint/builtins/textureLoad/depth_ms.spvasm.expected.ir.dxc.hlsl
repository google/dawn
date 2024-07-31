struct vertex_main_out {
  float4 tint_symbol_1_1;
};

struct vertex_main_outputs {
  float4 vertex_main_out_tint_symbol_1_1 : SV_Position;
};


Texture2DMS<float4> arg_0 : register(t0, space1);
static float4 tint_symbol_1 = (0.0f).xxxx;
void textureLoad_6273b1() {
  float res = 0.0f;
  Texture2DMS<float4> v = arg_0;
  int2 v_1 = int2((0).xx);
  res = float4(v.Load(v_1, int(1)).x, 0.0f, 0.0f, 0.0f)[0u];
}

void tint_symbol_2(float4 tint_symbol) {
  tint_symbol_1 = tint_symbol;
}

void vertex_main_1() {
  textureLoad_6273b1();
  tint_symbol_2((0.0f).xxxx);
}

vertex_main_out vertex_main_inner() {
  vertex_main_1();
  vertex_main_out v_2 = {tint_symbol_1};
  return v_2;
}

void fragment_main_1() {
  textureLoad_6273b1();
}

void fragment_main() {
  fragment_main_1();
}

void compute_main_1() {
  textureLoad_6273b1();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

vertex_main_outputs vertex_main() {
  vertex_main_out v_3 = vertex_main_inner();
  vertex_main_outputs v_4 = {v_3.tint_symbol_1_1};
  return v_4;
}

