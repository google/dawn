static uint x_1 = 0u;
static float4 x_4 = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const uint x_2 = x_1;
  return;
}

struct main_out {
  float4 x_4_1;
};
struct tint_symbol_1 {
  uint x_1_param : SV_InstanceID;
};
struct tint_symbol_2 {
  float4 x_4_1 : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const uint x_1_param = tint_symbol.x_1_param;
  x_1 = x_1_param;
  main_1();
  const main_out tint_symbol_3 = {x_4};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.x_4_1};
  return tint_symbol_4;
}
