static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const float4 x_23 = v;
  v = float4(float2(1.0f, 1.0f).x, float2(1.0f, 1.0f).y, x_23.z, x_23.w);
  const float4 x_25 = v;
  v = float4(x_25.x, x_25.y, float2(2.0f, 2.0f).x, float2(2.0f, 2.0f).y);
  if (all((v == float4(1.0f, 1.0f, 2.0f, 2.0f)))) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  }
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
