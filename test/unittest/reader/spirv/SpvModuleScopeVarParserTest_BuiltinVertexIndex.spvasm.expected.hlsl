static uint x_2 = 0u;
static float4 position = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  return;
}

struct main_out {
  float4 position_1;
};
struct tint_symbol_1 {
  uint x_2_param : SV_VertexID;
};
struct tint_symbol_2 {
  float4 position_1 : SV_Position;
};

main_out main_inner(uint x_2_param) {
  x_2 = x_2_param;
  main_1();
  const main_out tint_symbol_3 = {position};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const main_out inner_result = main_inner(tint_symbol.x_2_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.position_1 = inner_result.position_1;
  return wrapper_result;
}
