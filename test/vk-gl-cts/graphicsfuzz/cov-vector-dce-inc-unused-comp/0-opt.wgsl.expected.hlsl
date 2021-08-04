cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2 a = float2(0.0f, 0.0f);
  float2 b = float2(0.0f, 0.0f);
  a = float2(1.0f, 1.0f);
  const int x_38 = asint(x_6[0].x);
  if ((x_38 == 1)) {
    const float x_43 = a.x;
    a.x = (x_43 + 1.0f);
  }
  const float x_47 = a.y;
  b = (float2(x_47, x_47) + float2(2.0f, 3.0f));
  if (all((b == float2(3.0f, 4.0f)))) {
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
