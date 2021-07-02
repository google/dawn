struct tint_array_wrapper {
  int arr[6];
};

tint_array_wrapper tint_symbol_1(ByteAddressBuffer buffer, uint offset) {
  const tint_array_wrapper tint_symbol_2 = {{asint(buffer.Load((offset + 0u))), asint(buffer.Load((offset + 4u))), asint(buffer.Load((offset + 8u))), asint(buffer.Load((offset + 12u))), asint(buffer.Load((offset + 16u))), asint(buffer.Load((offset + 20u)))}};
  return tint_symbol_2;
}

ByteAddressBuffer sspp962805860buildInformation : register(t2, space0);

void main_1() {
  tint_array_wrapper orientation = (tint_array_wrapper)0;
  const tint_array_wrapper x_23 = tint_symbol_1(sspp962805860buildInformation, 36u);
  orientation.arr[0] = x_23.arr[0u];
  orientation.arr[1] = x_23.arr[1u];
  orientation.arr[2] = x_23.arr[2u];
  orientation.arr[3] = x_23.arr[3u];
  orientation.arr[4] = x_23.arr[4u];
  orientation.arr[5] = x_23.arr[5u];
  return;
}

void main() {
  main_1();
  return;
}
