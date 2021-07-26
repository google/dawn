warning: use of deprecated intrinsic
static float4 color_out = float4(0.0f, 0.0f, 0.0f, 0.0f);
Texture2D<float4> tint_symbol : register(t0, space0);
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float4 x_19 = gl_FragCoord;
  const float4 x_22 = tint_symbol.Load(int3(float2(x_19.x, x_19.y), 0));
  color_out = x_22;
  return;
}

struct main_out {
  float4 color_out_1;
};
struct tint_symbol_2 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_3 {
  float4 color_out_1 : SV_Target0;
};

tint_symbol_3 main(tint_symbol_2 tint_symbol_1) {
  const float4 gl_FragCoord_param = tint_symbol_1.gl_FragCoord_param;
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  const main_out tint_symbol_4 = {color_out};
  const tint_symbol_3 tint_symbol_5 = {tint_symbol_4.color_out_1};
  return tint_symbol_5;
}
