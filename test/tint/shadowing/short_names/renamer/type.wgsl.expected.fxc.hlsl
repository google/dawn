struct vec4f {
  int i;
};
struct tint_symbol_1 {
  uint VertexIndex : SV_VertexID;
};
struct tint_symbol_2 {
  float4 value : SV_Position;
};

float4 main_inner(uint VertexIndex) {
  vec4f s = {1};
  float f = float(s.i);
  bool b = bool(f);
  return (b ? (1.0f).xxxx : (0.0f).xxxx);
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  float4 inner_result = main_inner(tint_symbol.VertexIndex);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.value = inner_result;
  return wrapper_result;
}
