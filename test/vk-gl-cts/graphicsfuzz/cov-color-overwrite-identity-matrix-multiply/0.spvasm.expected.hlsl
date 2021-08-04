static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0, space0) {
  uint4 x_6[5];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float x_33 = gl_FragCoord.x;
  const uint scalar_offset = ((16u * uint(0))) / 4;
  const float x_35 = asfloat(x_6[scalar_offset / 4][scalar_offset % 4]);
  if ((x_33 > x_35)) {
    const float x_40 = asfloat(x_6[2].x);
    x_GLF_color = float4(x_40, x_40, x_40, x_40);
    const float x_43 = gl_FragCoord.y;
    if ((x_43 > x_35)) {
      const float x_48 = asfloat(x_6[4].x);
      x_GLF_color = float4(x_48, x_48, x_48, x_48);
    }
    const float x_51 = asfloat(x_6[3].x);
    x_GLF_color = float4(x_51, x_51, x_51, x_51);
  }
  const float x_54 = asfloat(x_6[1].x);
  x_GLF_color = float4(x_35, x_54, x_54, 10.0f);
  x_GLF_color = mul(x_GLF_color, float4x4(float4(x_35, 0.0f, 0.0f, 0.0f), float4(0.0f, x_35, 0.0f, 0.0f), float4(0.0f, 0.0f, x_35, 0.0f), float4(0.0f, 0.0f, 0.0f, x_35)));
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
