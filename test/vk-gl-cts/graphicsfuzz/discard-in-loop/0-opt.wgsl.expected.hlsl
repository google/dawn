static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void x_47() {
  discard;
}

void main_1() {
  while (true) {
    int x_30_phi = 0;
    bool x_48_phi = false;
    x_30_phi = 0;
    while (true) {
      int x_31 = 0;
      const int x_30 = x_30_phi;
      x_48_phi = false;
      if ((x_30 < 10)) {
      } else {
        break;
      }
      const float x_37 = gl_FragCoord.y;
      if ((x_37 < 0.0f)) {
        const float x_42 = gl_FragCoord.x;
        if ((x_42 < 0.0f)) {
          x_48_phi = false;
          break;
        } else {
          {
            x_31 = (x_30 + 1);
            x_30_phi = x_31;
          }
          continue;
        }
        {
          x_31 = (x_30 + 1);
          x_30_phi = x_31;
        }
        continue;
      }
      x_47();
      x_48_phi = true;
      break;
      {
        x_31 = (x_30 + 1);
        x_30_phi = x_31;
      }
    }
    if (x_48_phi) {
      break;
    }
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    break;
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
