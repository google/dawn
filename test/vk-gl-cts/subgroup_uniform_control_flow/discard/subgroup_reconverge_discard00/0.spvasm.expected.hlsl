static float3 position = float3(0.0f, 0.0f, 0.0f);
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  const float3 x_21 = position;
  gl_Position = float4(x_21.x, x_21.y, x_21.z, 1.0f);
  return;
}

struct main_out {
  float4 gl_Position;
};
struct tint_symbol_1 {
  float3 position_param : TEXCOORD0;
};
struct tint_symbol_2 {
  float4 gl_Position : SV_Position;
};

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float3 position_param = tint_symbol.position_param;
  position = position_param;
  main_1();
  const main_out tint_symbol_3 = {gl_Position};
  const tint_symbol_2 tint_symbol_4 = {tint_symbol_3.gl_Position};
  return tint_symbol_4;
}
