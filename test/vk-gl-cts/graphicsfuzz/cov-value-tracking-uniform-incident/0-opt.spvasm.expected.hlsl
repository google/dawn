cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 N = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 I = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 Nref = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  N = float4(1.0f, 2.0f, 3.0f, 4.0f);
  const float x_44 = asfloat(x_7[0].x);
  I = float4(4.0f, 87.589996338f, x_44, 92.510002136f);
  Nref = float4(17.049999237f, -6.099999905f, 4329.370605469f, 2.700000048f);
  v = faceforward(N, I, Nref);
  if (all((v == float4(-1.0f, -2.0f, -3.0f, -4.0f)))) {
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
