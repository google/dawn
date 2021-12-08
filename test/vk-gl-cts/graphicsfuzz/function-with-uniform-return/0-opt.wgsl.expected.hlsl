SKIP: FAILED

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float fx_() {
  const float x_50 = gl_FragCoord.y;
  if ((x_50 >= 0.0f)) {
    const float x_55 = asfloat(x_7[0].y);
    return x_55;
  }
  [loop] while (true) {
    if (true) {
    } else {
      break;
    }
    x_GLF_color = float4(1.0f, 1.0f, 1.0f, 1.0f);
  }
  return 0.0f;
}

void main_1() {
  float x2 = 0.0f;
  float B = 0.0f;
  float k0 = 0.0f;
  x2 = 1.0f;
  B = 1.0f;
  const float x_34 = fx_();
  x_GLF_color = float4(x_34, 0.0f, 0.0f, 1.0f);
  [loop] while (true) {
    if ((x2 > 2.0f)) {
    } else {
      break;
    }
    const float x_43 = fx_();
    const float x_44 = fx_();
    k0 = (x_43 - x_44);
    B = k0;
    x2 = B;
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
C:\src\tint\test\Shader@0x0000024AE2EDC600(13,17-20): error X3696: infinite loop detected - loop never exits

