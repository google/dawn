static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int GLF_live9r = 0;
  float g = 0.0f;
  while (true) {
    if (true) {
    } else {
      break;
    }
    if (true) {
      break;
    }
    const int x_32 = clamp(GLF_live9r, 0, 1);
  }
  g = 3.0f;
  x_GLF_color = float4(smoothstep(0.0f, 1.0f, g), 0.0f, 0.0f, 1.0f);
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
