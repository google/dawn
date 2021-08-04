static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};

void main_1() {
  float4 x_24 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
  const int x_26 = asint(x_6[0].x);
  if ((x_26 == 0)) {
    return;
  }
  x_24 = float4(1.0f, 0.0f, 0.0f, 1.0f);
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

float4 func_() {
  return float4(1.0f, 0.0f, 0.0f, 1.0f);
}
