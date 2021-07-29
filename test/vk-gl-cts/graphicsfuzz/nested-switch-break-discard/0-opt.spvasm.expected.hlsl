SKIP: FAILED

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  x_GLF_color = float4(0.0f, 0.0f, 0.0f, 1.0f);
  const float x_30 = asfloat(x_6[0].x);
  switch(int(x_30)) {
    case 0: {
      switch(1) {
        case 1: {
          const float x_38 = gl_FragCoord.y;
          if ((x_38 >= 0.0f)) {
            x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
            break;
          }
          discard;
          break;
        }
        default: {
          break;
        }
      }
      /* fallthrough */
    }
    case 42: {
      break;
    }
    default: {
      break;
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

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 gl_FragCoord_param = tint_symbol.gl_FragCoord_param;
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  const tint_symbol_2 tint_symbol_5 = {tint_symbol_3.x_GLF_color_1};
  return tint_symbol_5;
}
C:\src\tint\test\Shader@0x000001A268ED57F0(11,5): error X3533: non-empty case statements must have break or return
C:\src\tint\test\Shader@0x000001A268ED57F0(27,5): error X3537: Fall-throughs in switch statements are not allowed.

