static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};

void main_1() {
  int i = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  i = 0;
  {
    for(; (i < 10); i = (i + 1)) {
      x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
      const float x_39 = asfloat(x_6[0].y);
      if ((1.0f > x_39)) {
        x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
        if (true) {
          return;
        }
      }
    }
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
