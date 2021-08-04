static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static int expect = 0;

void main_1() {
  bool inbounds = false;
  bool x_31 = false;
  bool x_32_phi = false;
  const float x_24 = gl_FragCoord.x;
  const bool x_25 = (x_24 < 128.0f);
  x_32_phi = x_25;
  if (!(x_25)) {
    const float x_30 = gl_FragCoord.y;
    x_31 = (x_30 < 128.0f);
    x_32_phi = x_31;
  }
  inbounds = x_32_phi;
  expect = (inbounds ? 1 : -1);
  return;
}

struct main_out {
  int expect_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  int expect_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {expect};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.expect_1 = inner_result.expect_1;
  return wrapper_result;
}
