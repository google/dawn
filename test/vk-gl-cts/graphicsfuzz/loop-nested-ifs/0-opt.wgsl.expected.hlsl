static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int j = 0;
  float a = 0.0f;
  j = 0;
  {
    for(; (j < 2); j = (j + 1)) {
      if ((j < 1)) {
        x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
      }
      if ((j != 3)) {
        if ((j != 4)) {
          if ((j == 5)) {
            x_GLF_color.x = ldexp(1.0f, 2);
          } else {
            a = ldexp(1.0f, 2);
            x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
          }
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
