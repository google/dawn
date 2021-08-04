static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float nb_mod_f1_(inout float limit) {
  int x_injected_loop_counter = 0;
  int x_injected_loop_counter_1 = 0;
  const float x_37 = limit;
  if ((1.0f >= x_37)) {
    return 1.0f;
  }
  x_injected_loop_counter = 0;
  while (true) {
    const bool x_42 = (0 < 2);
    x_injected_loop_counter_1 = 0;
    while (true) {
      const bool x_47 = (0 < 1);
      return 1.0f;
    }
  }
  return 0.0f;
}

void main_1() {
  float param = 0.0f;
  const float x_34 = gl_FragCoord.x;
  param = x_34;
  const float x_35 = nb_mod_f1_(param);
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
  const main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
