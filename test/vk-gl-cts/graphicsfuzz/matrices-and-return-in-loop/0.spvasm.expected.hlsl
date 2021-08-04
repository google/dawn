static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float3 f_mf22_(inout float2x2 m) {
  while (true) {
    return float3(1.0f, 1.0f, 1.0f);
  }
  return float3(0.0f, 0.0f, 0.0f);
}

void main_1() {
  float2x2 param = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_38_phi = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  const float x_34 = gl_FragCoord.x;
  x_38_phi = float2x2(float2(0.0f, 0.0f), float2(0.0f, 0.0f));
  if ((x_34 >= 0.0f)) {
    x_38_phi = float2x2(float2(1.0f, 0.0f), float2(0.0f, 1.0f));
  }
  const float2x2 x_38 = x_38_phi;
  param = mul(x_38, x_38);
  const float3 x_40 = f_mf22_(param);
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
