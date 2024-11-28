struct tint_symbol {
  int tint_symbol_1;
};

struct tint_symbol_4_outputs {
  float4 tint_symbol_2 : SV_Position;
};

struct tint_symbol_4_inputs {
  uint tint_symbol_5 : SV_VertexID;
};


float4 tint_symbol_4_inner(uint tint_symbol_5) {
  tint_symbol tint_symbol_6 = {int(1)};
  float tint_symbol_7 = float(tint_symbol_6.tint_symbol_1);
  bool tint_symbol_8 = bool(tint_symbol_7);
  return ((tint_symbol_8) ? ((1.0f).xxxx) : ((0.0f).xxxx));
}

tint_symbol_4_outputs tint_symbol_4(tint_symbol_4_inputs inputs) {
  tint_symbol_4_outputs v = {tint_symbol_4_inner(inputs.tint_symbol_5)};
  return v;
}

