static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int ll = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  const float x_30 = gl_FragCoord.x;
  if ((int(x_30) < 2000)) {
  } else {
    ll = 0;
    while (true) {
      const float x_41 = gl_FragCoord.x;
      if ((x_41 < 0.0f)) {
        discard;
      }
      if ((ll >= 5)) {
        break;
      }
      {
        ll = (ll + 1);
      }
    }
    const float x_49 = gl_FragCoord.x;
    if ((int(x_49) >= 2000)) {
      discard;
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
