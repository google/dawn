int vec4f() {
  return 0;
}

float vec2f(int i) {
  return float(i);
}

bool vec2i(float f) {
  return bool(f);
}

struct tint_symbol_1 {
  uint VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 main_inner(uint VertexIndex) {
  const float4 tint_symbol_3 = (0.0f).xxxx;
  const float4 tint_symbol_4 = (1.0f).xxxx;
  const int tint_symbol_5 = vec4f();
  const float tint_symbol_6 = vec2f(tint_symbol_5);
  const bool tint_symbol_7 = vec2i(tint_symbol_6);
  return (tint_symbol_7 ? tint_symbol_4 : tint_symbol_3);
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  const float4 inner_result = main_inner(tint_symbol.VertexIndex);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
