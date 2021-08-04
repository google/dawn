static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};

void main_1() {
  int j = 0;
  float x_41 = 0.0f;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  while (true) {
    const float x_32 = asfloat(x_6[0].x);
    j = int(x_32);
    while (true) {
      if ((j < 2)) {
      } else {
        break;
      }
      return;
    }
    {
      x_41 = asfloat(x_6[0].y);
      if ((0.0f > x_41)) {
      } else {
        break;
      }
    }
  }
  const int x_43 = int(x_41);
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
