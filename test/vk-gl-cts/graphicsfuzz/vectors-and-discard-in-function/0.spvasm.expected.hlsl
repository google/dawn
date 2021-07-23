static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

int func_() {
  float2 coord = float2(0.0f, 0.0f);
  float tmp3 = 0.0f;
  float tmp2[1] = (float[1])0;
  float4 tmp = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float x_48 = 0.0f;
  coord = float2(1.0f, 1.0f);
  const float x_41 = coord.y;
  if ((int(x_41) < 180)) {
    x_48 = tmp2[0];
    tmp3 = x_48;
  } else {
    discard;
  }
  tmp = float4(x_48, x_48, x_48, x_48);
  return 1;
}

void main_1() {
  const int x_9 = func_();
  if ((x_9 == 1)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
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
  const tint_symbol tint_symbol_2 = {tint_symbol_1.x_GLF_color_1};
  return tint_symbol_2;
}
