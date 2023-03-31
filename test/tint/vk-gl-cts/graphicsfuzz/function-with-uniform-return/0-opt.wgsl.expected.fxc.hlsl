SKIP: FAILED

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float fx_() {
  const float x_50 = gl_FragCoord.y;
  if ((x_50 >= 0.0f)) {
    const float x_55 = asfloat(x_7[0].y);
    return x_55;
  }
  while (true) {
    if (true) {
    } else {
      break;
    }
    x_GLF_color = (1.0f).xxxx;
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
  while (true) {
    const float x_40 = x2;
    if ((x_40 > 2.0f)) {
    } else {
      break;
    }
    const float x_43 = fx_();
    const float x_44 = fx_();
    k0 = (x_43 - x_44);
    const float x_46 = k0;
    B = x_46;
    const float x_47 = B;
    x2 = x_47;
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
