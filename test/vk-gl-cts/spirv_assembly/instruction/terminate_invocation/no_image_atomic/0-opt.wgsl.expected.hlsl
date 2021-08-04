static float3 x_2 = float3(0.0f, 0.0f, 0.0f);
static int x_3 = 0;
static int x_4 = 0;
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  gl_Position = float4(x_2, 1.0f);
  x_4 = x_3;
  return;
}

struct main_out {
  int x_4_1;
  float4 gl_Position;
};
struct tint_symbol_1 {
  float3 x_2_param : TEXCOORD0;
  int x_3_param : TEXCOORD1;
};
struct tint_symbol_2 {
  int x_4_1 : TEXCOORD0;
  float4 gl_Position : SV_Position;
};

main_out main_inner(float3 x_2_param, int x_3_param) {
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  const main_out tint_symbol_3 = {x_4, gl_Position};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.x_2_param, tint_symbol.x_3_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_4_1 = inner_result.x_4_1;
  wrapper_result.gl_Position = inner_result.gl_Position;
  return wrapper_result;
}
