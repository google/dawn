static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4x3 x_37 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float4x3 x_38_phi = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float3 x_48_phi = float3(0.0f, 0.0f, 0.0f);
  const float x_32 = gl_FragCoord.y;
  if ((x_32 < 1.0f)) {
    x_38_phi = float4x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), float3(0.0f, 0.0f, 0.0f));
  } else {
    x_37 = transpose(float3x4(float4(1.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 1.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 1.0f, 0.0f)));
    x_38_phi = x_37;
  }
  const float x_40 = transpose(x_38_phi)[0u].y;
  while (true) {
    if ((x_40 > 1.0f)) {
      x_48_phi = float3(0.0f, 0.0f, 0.0f);
      break;
    }
    x_48_phi = float3(1.0f, 0.0f, 0.0f);
    break;
  }
  const float3 x_48 = x_48_phi;
  x_GLF_color = float4(x_48.x, x_48.y, x_48.z, 1.0f);
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
