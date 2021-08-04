static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float x_30 = 0.0f;
  uint foo = 0u;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  const float x_32 = gl_FragCoord.x;
  if ((x_32 > -1.0f)) {
    const float x_38 = x_GLF_color.x;
    x_30 = x_38;
  } else {
    const uint x_7 = (foo - asuint(1));
    foo = x_7;
    x_30 = float((178493u + x_7));
  }
  x_GLF_color.x = x_30;
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
