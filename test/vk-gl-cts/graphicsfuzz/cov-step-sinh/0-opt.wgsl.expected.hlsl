static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 v1 = float2(0.0f, 0.0f);
  float2 v2 = float2(0.0f, 0.0f);
  v1 = float2(1.0f, -1.0f);
  v2 = step(float2(0.400000006f, 0.400000006f), sinh(v1));
  const float x_27 = v2.x;
  const float x_29 = v2.y;
  const float x_31 = v2.y;
  const float x_33 = v2.x;
  x_GLF_color = float4(x_27, x_29, x_31, x_33);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_2 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_2;
}
