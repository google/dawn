static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int out_data = 0;

void main_1() {
  bool x_is_odd = false;
  bool y_is_odd = false;
  const float x_24 = gl_FragCoord.x;
  x_is_odd = ((int(x_24) & 1) == 1);
  const float x_29 = gl_FragCoord.y;
  y_is_odd = ((int(x_29) & 1) == 1);
  bool tint_tmp = x_is_odd;
  if (!tint_tmp) {
    tint_tmp = y_is_odd;
  }
  out_data = ((tint_tmp) ? 1 : 0);
  return;
}

struct main_out {
  int out_data_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  int out_data_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {out_data};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.out_data_1 = inner_result.out_data_1;
  return wrapper_result;
}
