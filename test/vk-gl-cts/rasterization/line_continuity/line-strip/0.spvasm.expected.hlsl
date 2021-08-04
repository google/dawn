static float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  color = float4(1.0f, 1.0f, 1.0f, 1.0f);
  return;
}

struct main_out {
  float4 color_1;
};
struct tint_symbol {
  float4 color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.color_1 = inner_result.color_1;
  return wrapper_result;
}
