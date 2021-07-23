static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int i = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  i = 1;
  {
    for(; (i < 2); i = (i + 1)) {
      const float x_37 = gl_FragCoord.y;
      if ((x_37 < 0.0f)) {
        const float x_42 = gl_FragCoord.x;
        if ((x_42 < 0.0f)) {
          x_GLF_color = float4(226.695999146f, 1.0f, 1.0f, 1.0f);
        }
        continue;
      }
      return;
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
