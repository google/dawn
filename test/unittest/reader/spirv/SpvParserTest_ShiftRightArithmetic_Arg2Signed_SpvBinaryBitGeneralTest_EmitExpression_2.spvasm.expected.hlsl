void main_1() {
  const uint2 x_1 = asuint((asint(uint2(10u, 20u)) >> asuint(int2(30, 40))));
  return;
}

void main() {
  main_1();
  return;
}
