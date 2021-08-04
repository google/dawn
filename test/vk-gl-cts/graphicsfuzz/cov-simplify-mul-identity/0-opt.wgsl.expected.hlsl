cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 res = float4(0.0f, 0.0f, 0.0f, 0.0f);
  v = float4(8.399999619f, -864.664978027f, 945.41998291f, 1.0f);
  const float x_31 = asfloat(x_7[0].x);
  res = mul(v, float4x4(float4(x_31, 0.0f, 0.0f, 0.0f), float4(0.0f, x_31, 0.0f, 0.0f), float4(0.0f, 0.0f, x_31, 0.0f), float4(0.0f, 0.0f, 0.0f, x_31)));
  if ((distance(v, res) < 0.01f)) {
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
