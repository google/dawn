static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2x2 m = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  m = (transpose(float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f))) * (1.0f / 2.0f));
  const float2x2 x_33 = m;
  if ((all((x_33[0u] == float2x2(float2(0.5f, 1.5f), float2(1.0f, 2.0f))[0u])) & all((x_33[1u] == float2x2(float2(0.5f, 1.5f), float2(1.0f, 2.0f))[1u])))) {
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
