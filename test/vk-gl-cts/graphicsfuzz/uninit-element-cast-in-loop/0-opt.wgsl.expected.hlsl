cbuffer cbuffer_x_5 : register(b0, space0) {
  uint4 x_5[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float x_30 = 0.0f;
  float x_32_phi = 0.0f;
  x_32_phi = 0.0f;
  while (true) {
    float x_33_phi = 0.0f;
    x_33_phi = x_32_phi;
    const float x_33 = x_33_phi;
    const float x_39 = asfloat(x_5[0].x);
    const float x_41 = asfloat(x_5[0].y);
    if ((x_39 < x_41)) {
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
      return;
    } else {
      {
        x_32_phi = x_33;
      }
      continue;
    }
    {
      x_32_phi = x_33;
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
