cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  f = 1.0f;
  while (true) {
    const float x_31 = asfloat(x_6[0].x);
    f = (f + x_31);
    {
      const float x_34 = f;
      const float x_36 = asfloat(x_6[0].x);
      if ((10.0f > clamp(x_34, 8.0f, (9.0f + x_36)))) {
      } else {
        break;
      }
    }
  }
  if ((f == 10.0f)) {
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
