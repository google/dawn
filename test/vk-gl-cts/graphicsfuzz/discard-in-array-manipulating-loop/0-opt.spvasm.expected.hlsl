static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float data[10] = (float[10])0;
  int i = 0;
  const float tint_symbol_4[10] = {0.100000001f, 0.200000003f, 0.300000012f, 0.400000006f, 0.5f, 0.600000024f, 0.699999988f, 0.800000012f, 0.899999976f, 1.0f};
  data = tint_symbol_4;
  i = 0;
  {
    for(; (i < 10); i = (i + 1)) {
      const float x_50 = gl_FragCoord.x;
      if ((x_50 < 0.0f)) {
        discard;
      }
      const float x_55 = data[i];
      data[0] = x_55;
    }
  }
  const float x_58 = data[0];
  x_GLF_color = float4(x_58, 0.0f, 0.0f, 1.0f);
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
  const tint_symbol_2 tint_symbol_5 = {tint_symbol_3.x_GLF_color_1};
  return tint_symbol_5;
}
