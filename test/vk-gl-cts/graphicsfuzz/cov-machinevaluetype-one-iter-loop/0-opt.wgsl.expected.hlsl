cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int a = 0;
  int i = 0;
  a = 0;
  i = 0;
  while (true) {
    const int x_33 = i;
    const float x_35 = asfloat(x_7[0].y);
    if ((x_33 < int(x_35))) {
    } else {
      break;
    }
    if ((a > 0)) {
      break;
    }
    const float x_44 = asfloat(x_7[0].y);
    a = ((int(x_44) * 2) / 2);
    {
      i = (i + 1);
    }
  }
  if ((a == 1)) {
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

tint_symbol main() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  const tint_symbol tint_symbol_3 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_3;
}
