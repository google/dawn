static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  float data[1] = (float[1])0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  i = 0;
  {
    for(; (i < 1); i = (i + 1)) {
      const float x_40 = data[i];
      const float x_42 = data[0];
      if ((x_40 < x_42)) {
        if (false) {
          if ((float(i) >= 1.0f)) {
          }
        }
        switch(0) {
          case 1: {
            while (true) {
              if (true) {
              } else {
                break;
              }
            }
            while (true) {
            }
            /* fallthrough */
            {
              data[0] = 2.0f;
            }
            break;
          }
          case 0: {
            data[0] = 2.0f;
            break;
          }
          default: {
            break;
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
