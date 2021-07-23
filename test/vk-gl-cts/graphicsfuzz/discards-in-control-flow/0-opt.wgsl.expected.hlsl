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

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 gl_FragCoord_param = tint_symbol.gl_FragCoord_param;
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_3 = {x_GLF_color};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.x_GLF_color_1};
  return tint_symbol_4;
}
