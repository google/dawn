cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  v = float4(2.0f, 3.0f, 4.0f, 5.0f);
  const float x_40 = asfloat(x_6[0].y);
  const float2 x_42 = (bool2(true, false) ? float2(1.0f, x_40) : float2(2.0f, 6.0f));
  const float4 x_43 = v;
  v = float4(x_42.x, x_42.y, x_43.z, x_43.w);
  if (all((v == float4(1.0f, 6.0f, 4.0f, 5.0f)))) {
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
