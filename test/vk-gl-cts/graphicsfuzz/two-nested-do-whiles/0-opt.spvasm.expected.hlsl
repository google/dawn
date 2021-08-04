static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  int i_1 = 0;
  int i_2 = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  i = 0;
  const float x_35 = asfloat(x_7[0].y);
  if ((x_35 < 0.0f)) {
  } else {
    bool x_42 = false;
    const float x_41 = gl_FragCoord.y;
    x_42 = (x_41 < -1.0f);
    if (x_42) {
    } else {
      while (true) {
        if ((i >= 256)) {
          break;
        }
        while (true) {
          i_1 = 0;
          {
            for(; (i_1 < 1); i_1 = (i_1 + 1)) {
              if (x_42) {
                i_2 = 0;
                {
                  for(; (i_2 < 1); i_2 = (i_2 + 1)) {
                  }
                }
                continue;
              }
              return;
            }
          }
          {
            if (false) {
            } else {
              break;
            }
          }
        }
        {
          if (false) {
          } else {
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
