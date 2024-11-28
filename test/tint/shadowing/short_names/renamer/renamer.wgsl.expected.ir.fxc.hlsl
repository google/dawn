struct tint_symbol_outputs {
  float4 tint_symbol_2 : SV_Position;
};

struct tint_symbol_inputs {
  uint tint_symbol_1 : SV_VertexID;
};


float4 tint_symbol_inner(uint tint_symbol_1) {
  return float4(0.0f, 0.0f, 0.0f, 1.0f);
}

tint_symbol_outputs tint_symbol(tint_symbol_inputs inputs) {
  tint_symbol_outputs v = {tint_symbol_inner(inputs.tint_symbol_1)};
  return v;
}

