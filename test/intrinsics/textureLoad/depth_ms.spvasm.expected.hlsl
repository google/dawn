Texture2DMS<float4> arg_0 : register(t0, space1);
static float4 tint_symbol_1 = float4(0.0f, 0.0f, 0.0f, 0.0f);

void textureLoad_6273b1() {
  float res = 0.0f;
  const float4 x_17 = float4(arg_0.Load(int3(0, 0, 0), 1).x, 0.0f, 0.0f, 0.0f);
  res = x_17.x;
  return;
}

void tint_symbol_2(float4 tint_symbol) {
  tint_symbol_1 = tint_symbol;
  return;
}

void vertex_main_1() {
  textureLoad_6273b1();
  tint_symbol_2(float4(0.0f, 0.0f, 0.0f, 0.0f));
  return;
}

struct vertex_main_out {
  float4 tint_symbol_1_1;
};
struct tint_symbol_3 {
  float4 tint_symbol_1_1 : SV_Position;
};

tint_symbol_3 vertex_main() {
  vertex_main_1();
  const vertex_main_out tint_symbol_4 = {tint_symbol_1};
  const tint_symbol_3 tint_symbol_5 = {tint_symbol_4.tint_symbol_1_1};
  return tint_symbol_5;
}

void fragment_main_1() {
  textureLoad_6273b1();
  return;
}

void fragment_main() {
  fragment_main_1();
  return;
}

void compute_main_1() {
  textureLoad_6273b1();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
  return;
}
