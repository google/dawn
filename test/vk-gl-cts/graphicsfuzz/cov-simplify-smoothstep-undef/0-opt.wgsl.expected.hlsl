static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float f = 0.0f;
  bool x_49 = false;
  bool x_50_phi = false;
  const float x_31 = gl_FragCoord.x;
  f = x_31;
  f = (f + asfloat(0xff800000u));
  if (((5.0f / f) == 0.0f)) {
    f = (f + 1.0f);
  }
  const bool x_42 = isnan(f);
  x_50_phi = x_42;
  if (!(x_42)) {
    const float x_46 = f;
    const float x_48 = asfloat(x_7[0].x);
    x_49 = (x_46 != x_48);
    x_50_phi = x_49;
  }
  if (x_50_phi) {
    f = 0.0f;
  }
  if ((f == 0.0f)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
  const main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.gl_FragCoord_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
