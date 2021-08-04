static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float func_() {
  float x = 0.0f;
  x = 2.0f;
  const float x_35 = gl_FragCoord.x;
  if ((x_35 == 12.0f)) {
    const float x_40 = gl_FragCoord.y;
    if ((x_40 == 13.0f)) {
      x = (x + 1.0f);
    }
    return x;
  }
  return 1.0f;
}

void main_1() {
  if (false) {
    const float x_31 = func_();
    x_GLF_color = float4(x_31, x_31, x_31, x_31);
  } else {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
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
