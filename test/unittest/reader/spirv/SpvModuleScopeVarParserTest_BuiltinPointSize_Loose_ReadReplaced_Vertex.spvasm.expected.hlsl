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

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_2};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_2_1 = inner_result.x_2_1;
  return wrapper_result;
}
