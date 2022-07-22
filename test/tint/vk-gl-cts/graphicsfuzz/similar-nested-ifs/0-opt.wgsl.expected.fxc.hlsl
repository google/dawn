SKIP: FAILED

cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float gv = 0.0f;
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float lv = 0.0f;
  float x_43 = 0.0f;
  int GLF_live5r = 0;
  int GLF_live5_looplimiter6 = 0;
  const float x_45 = asfloat(x_7[0].y);
  if ((1.0f > x_45)) {
    x_43 = abs(gv);
  } else {
    x_43 = 260.0f;
  }
  lv = x_43;
  if ((int(lv) < 250)) {
    if ((int(lv) < 180)) {
      const float x_65 = clamp(lv, 1.0f, 1.0f);
    } else {
      const float x_67 = gl_FragCoord.y;
      if ((x_67 < 0.0f)) {
        if ((int(lv) < 210)) {
          [loop] while (true) {
            {
              if (true) {
              } else {
                break;
              }
            }
          }
        }
        GLF_live5r = 0;
        [loop] while (true) {
          if (true) {
          } else {
            break;
          }
          if ((GLF_live5_looplimiter6 >= 6)) {
            break;
          }
          GLF_live5_looplimiter6 = (GLF_live5_looplimiter6 + 1);
        }
      }
    }
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
C:\src\tint\test\Shader@0x0000023F9486F180(27,18-29): warning X3557: loop doesn't seem to do anything, consider removing [loop]
C:\src\tint\test\Shader@0x0000023F9486F180(27,18-29): warning X3551: infinite loop detected - loop writes no values
C:\src\tint\test\Shader@0x0000023F9486F180(27,25-28): error X3696: infinite loop detected - loop never exits

