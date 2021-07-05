typedef int tint_symbol_1_ret[6];
tint_symbol_1_ret tint_symbol_1(ByteAddressBuffer buffer, uint offset) {
  const int tint_symbol_2[6] = {asint(buffer.Load((offset + 0u))), asint(buffer.Load((offset + 4u))), asint(buffer.Load((offset + 8u))), asint(buffer.Load((offset + 12u))), asint(buffer.Load((offset + 16u))), asint(buffer.Load((offset + 20u)))};
  return tint_symbol_2;
}

ByteAddressBuffer sspp962805860buildInformation : register(t2, space0);

void main_1() {
  int orientation[6] = (int[6])0;
  const int x_23[6] = tint_symbol_1(sspp962805860buildInformation, 36u);
  orientation[0] = x_23[0u];
  orientation[1] = x_23[1u];
  orientation[2] = x_23[2u];
  orientation[3] = x_23[3u];
  orientation[4] = x_23[4u];
  orientation[5] = x_23[5u];
  return;
}

void main() {
  main_1();
  return;
}
