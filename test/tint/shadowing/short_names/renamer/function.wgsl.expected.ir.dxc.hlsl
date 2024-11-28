struct tint_symbol_5_outputs {
  float4 tint_symbol_7 : SV_Position;
};

struct tint_symbol_5_inputs {
  uint tint_symbol_6 : SV_VertexID;
};


int tint_symbol() {
  return int(0);
}

float tint_symbol_1(int tint_symbol_2) {
  return float(tint_symbol_2);
}

bool tint_symbol_3(float tint_symbol_4) {
  return bool(tint_symbol_4);
}

float4 tint_symbol_5_inner(uint tint_symbol_6) {
  return ((tint_symbol_3(tint_symbol_1(tint_symbol()))) ? ((1.0f).xxxx) : ((0.0f).xxxx));
}

tint_symbol_5_outputs tint_symbol_5(tint_symbol_5_inputs inputs) {
  tint_symbol_5_outputs v = {tint_symbol_5_inner(inputs.tint_symbol_6)};
  return v;
}

