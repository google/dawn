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

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {color};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.color_1};
  return tint_symbol_2;
}
