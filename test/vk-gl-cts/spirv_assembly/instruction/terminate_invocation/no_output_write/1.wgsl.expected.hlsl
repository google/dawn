static float4 x_2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int x_3 = 0;
static int x_4 = 0;

void main_1() {
  const float4 x_16 = x_2;
  if (((((int(x_16.x) & 1) + (int(x_16.y) & 1)) + x_3) == int(x_16.z))) {
  }
  x_4 = 1;
  return;
}

struct main_out {
  int x_4_1;
};
struct tint_symbol_1 {
  int x_3_param : TEXCOORD0;
  float4 x_2_param : SV_Position;
};
struct tint_symbol_2 {
  int x_4_1 : SV_Target0;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 x_2_param = tint_symbol.x_2_param;
  const int x_3_param = tint_symbol.x_3_param;
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  const main_out tint_symbol_3 = {x_4};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.x_4_1};
  return tint_symbol_4;
}
