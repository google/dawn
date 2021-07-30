static int x_4 = 0;
static float4 position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const int x_2 = x_4;
  return;
}

struct main_out {
  float4 position_1;
};
struct tint_symbol_1 {
  uint x_4_param : SV_InstanceID;
};
struct tint_symbol_2 {
  float4 position_1 : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const uint x_4_param = tint_symbol.x_4_param;
  x_4 = asint(x_4_param);
  main_1();
  const main_out tint_symbol_3 = {position};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.position_1};
  return tint_symbol_4;
}
