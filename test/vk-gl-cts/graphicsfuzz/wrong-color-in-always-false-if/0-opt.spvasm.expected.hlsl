cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float height = 0.0f;
  height = 256.0f;
  const float x_40 = asfloat(x_6[0].y);
  if ((x_40 < 0.0f)) {
    x_GLF_color = lerp(float4(30.180000305f, 8840.0f, 469.970001221f, 18.239999771f), float4(9.899999619f, 0.100000001f, 1169.538696289f, 55.790000916f), float4(7612.9453125f, 797.010986328f, height, 9.0f));
  }
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  const main_out tint_symbol_2 = {x_GLF_color};
  return tint_symbol_2;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
