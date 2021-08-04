static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  while (true) {
    bool x_46_phi = false;
    while (true) {
      const float x_37 = gl_FragCoord.x;
      if ((x_37 < 0.0f)) {
        const float x_42 = asfloat(x_6[0].y);
        if ((1.0f > x_42)) {
          discard;
        } else {
          {
            x_46_phi = false;
            if (false) {
            } else {
              break;
            }
          }
          continue;
        }
        {
          x_46_phi = false;
          if (false) {
          } else {
            break;
          }
        }
        continue;
      }
      x_46_phi = true;
      break;
      {
        x_46_phi = false;
        if (false) {
        } else {
          break;
        }
      }
    }
    if (x_46_phi) {
      break;
    }
    break;
  }
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
