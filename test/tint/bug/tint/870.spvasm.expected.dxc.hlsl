ByteAddressBuffer sspp962805860buildInformation : register(t2, space0);

typedef int tint_symbol_ret[6];
tint_symbol_ret tint_symbol(ByteAddressBuffer buffer, uint offset) {
  int arr[6] = (int[6])0;
  {
    [loop] for(uint i = 0u; (i < 6u); i = (i + 1u)) {
      arr[i] = asint(buffer.Load((offset + (i * 4u))));
    }
  }
  return arr;
}

void main_1() {
  int orientation[6] = (int[6])0;
  const int x_23[6] = tint_symbol(sspp962805860buildInformation, 36u);
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
