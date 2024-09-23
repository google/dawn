struct tint_symbol_1_inputs {
  uint tint_symbol_2 : SV_GroupIndex;
};


ByteAddressBuffer tint_symbol : register(t0);
void tint_symbol_1_inner(uint tint_symbol_2) {
  int tint_symbol_3 = int(0);
  int tint_symbol_4 = int(0);
  int tint_symbol_5 = int(0);
  uint v = 0u;
  tint_symbol.GetDimensions(v);
  uint tint_symbol_6 = tint_symbol.Load((0u + (uint(min(tint_symbol_2, ((v / 4u) - 1u))) * 4u)));
}

[numthreads(1, 1, 1)]
void tint_symbol_1(tint_symbol_1_inputs inputs) {
  tint_symbol_1_inner(inputs.tint_symbol_2);
}

