int vec4f() {
  return 0;
}

float vec2f(int i) {
  return float(i);
}

bool vec2i(float f) {
  return bool(f);
}

struct tint_symbol_6 {
  uint VertexIndex : SV_VertexID;
};
struct tint_symbol_7 {
  float4 value : SV_Position;
};

float4 main_inner(uint VertexIndex) {
  float4 tint_symbol = (0.0f).xxxx;
  float4 tint_symbol_1 = (1.0f).xxxx;
  int tint_symbol_2 = vec4f();
  float tint_symbol_3 = vec2f(tint_symbol_2);
  bool tint_symbol_4 = vec2i(tint_symbol_3);
  return (tint_symbol_4 ? tint_symbol_1 : tint_symbol);
}

tint_symbol_7 main(tint_symbol_6 tint_symbol_5) {
  float4 inner_result = main_inner(tint_symbol_5.VertexIndex);
  tint_symbol_7 wrapper_result = (tint_symbol_7)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
