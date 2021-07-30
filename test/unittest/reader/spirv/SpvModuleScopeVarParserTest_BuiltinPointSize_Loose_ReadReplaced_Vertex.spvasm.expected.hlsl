static float4 x_2 = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float x_900 = 0.0f;

void main_1() {
  x_900 = 1.0f;
  return;
}

struct main_out {
  float4 x_2_1;
};
struct tint_symbol {
  float4 x_2_1 : SV_Position;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_2};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.x_2_1};
  return tint_symbol_2;
}
