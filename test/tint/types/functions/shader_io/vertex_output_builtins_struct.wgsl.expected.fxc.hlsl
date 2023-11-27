struct VertexOutputs {
  float4 position;
};
struct tint_symbol {
  float4 position : SV_Position;
};

VertexOutputs main_inner() {
  const VertexOutputs tint_symbol_1 = {float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return tint_symbol_1;
}

tint_symbol main() {
  const VertexOutputs inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.position = inner_result.position;
  return wrapper_result;
}
