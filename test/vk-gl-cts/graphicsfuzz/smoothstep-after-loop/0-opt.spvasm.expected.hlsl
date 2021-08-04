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

main_out main_inner() {
  main_1();
  const main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  const main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
