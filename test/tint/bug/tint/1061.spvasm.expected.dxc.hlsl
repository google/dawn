cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  float4 v = float4(0.0f, 0.0f, 0.0f, 0.0f);
  f = 1.0f;
  const float x_33 = f;
  const float x_35 = f;
  const float x_37 = f;
  const float x_39 = f;
  v = float4(sin(x_33), cos(x_35), exp2(x_37), log(x_39));
  const float4 x_42 = v;
  const float4 x_44 = asfloat(x_7[0]);
  if ((distance(x_42, x_44) < 0.100000001f)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = (0.0f).xxxx;
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
