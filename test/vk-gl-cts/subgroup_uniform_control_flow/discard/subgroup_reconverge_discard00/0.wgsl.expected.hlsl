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

main_out main_inner(float3 position_param) {
  position = position_param;
  main_1();
  const main_out tint_symbol_3 = {gl_Position};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.position_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.gl_Position = inner_result.gl_Position;
  return wrapper_result;
}
