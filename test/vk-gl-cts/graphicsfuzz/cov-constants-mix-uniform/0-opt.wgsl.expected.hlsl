cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 mixed = float2(0.0f, 0.0f);
  const float2 x_30 = asfloat(x_6[0].xy);
  mixed = lerp(float2(1.0f, 1.0f), x_30, float2(0.5f, 0.5f));
  if (all((mixed == float2(1.0f, 1.0f)))) {
    const float x_40 = mixed.x;
    x_GLF_color = float4(x_40, 0.0f, 0.0f, 1.0f);
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
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
