struct S {
  int f1;
  float2x2 f2;
};

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float2x2 x_41 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  int x_6 = 0;
  float2x2 x_42 = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2x2 x_49_phi = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  const float x_44 = gl_FragCoord.x;
  if ((x_44 < 0.0f)) {
    x_42 = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
    x_49_phi = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
  } else {
    x_42 = float2x2(float2(0.5f, -0.5f), float2(-0.5f, 0.5f));
    x_49_phi = float2x2(float2(0.5f, -0.5f), float2(-0.5f, 0.5f));
  }
  const S x_51 = {1, transpose(x_49_phi)};
  const int x_52 = x_51.f1;
  x_6 = x_52;
  x_41 = x_51.f2;
  const float2x2 x_56 = x_41;
  const float2x2 x_59 = x_41;
  const float2x2 x_63 = x_41;
  x_GLF_color = float4(float(x_52), (x_56[0u].x + x_59[1u].x), (x_63[0u].y + x_41[1u].y), float(x_52));
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
