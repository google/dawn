RWByteAddressBuffer myvar : register(u0, space0);

void main_1() {
  uint tint_symbol_1 = 0u;
  myvar.GetDimensions(tint_symbol_1);
  const uint tint_symbol_2 = ((tint_symbol_1 - 4u) / 4u);
  const uint x_1 = tint_symbol_2;
  return;
}

void main() {
  main_1();
  return;
}
