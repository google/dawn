static int g = 0;
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  g = 0;
  while (true) {
    const int x_8 = g;
    const float x_46 = asfloat(x_6[0].x);
    if ((x_8 < int((x_46 + 2.0f)))) {
    } else {
      break;
    }
    g = (g + 1);
  }
  a = g;
  while (true) {
    const int x_12 = g;
    const float x_56 = asfloat(x_6[0].y);
    if ((x_12 < int(x_56))) {
    } else {
      break;
    }
    g = (g + 1);
  }
  a = a;
  if ((a == 2)) {
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
