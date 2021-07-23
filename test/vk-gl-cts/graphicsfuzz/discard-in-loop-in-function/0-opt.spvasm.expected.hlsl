cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void f_() {
  while (true) {
    const float x_35 = asfloat(x_7[0].y);
    if ((1.0f > x_35)) {
      const float x_40 = gl_FragCoord.y;
      if ((x_40 < 0.0f)) {
        {
          if (false) {
          } else {
            break;
          }
        }
        continue;
      } else {
        {
          if (false) {
          } else {
            break;
          }
        }
        continue;
      }
      {
        if (false) {
        } else {
          break;
        }
      }
      continue;
    }
    discard;
    {
      if (false) {
      } else {
        break;
      }
    }
  }
  return;
}

void main_1() {
  f_();
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

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 gl_FragCoord_param = tint_symbol.gl_FragCoord_param;
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  const tint_symbol_2 tint_symbol_5 = {tint_symbol_3.x_GLF_color_1};
  return tint_symbol_5;
}
