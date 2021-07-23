cbuffer cbuffer_x_7 : register(b0, space0) {
  uint4 x_7[1];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 computePoint_() {
  const float x_48 = asfloat(x_7[0].x);
  const float x_50 = asfloat(x_7[0].y);
  if ((x_48 > x_50)) {
    discard;
  }
  return float3(0.0f, 0.0f, 0.0f);
}

void main_1() {
  bool x_34 = false;
  while (true) {
    const float3 x_36 = computePoint_();
    const float x_41 = gl_FragCoord.x;
    if ((x_41 < 0.0f)) {
      x_34 = true;
      break;
    }
    const float3 x_45 = computePoint_();
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    x_34 = true;
    break;
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
  const tint_symbol_2 tint_symbol_5 = {tint_symbol_3.x_GLF_color_1};
  return tint_symbol_5;
}
