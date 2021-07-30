void main_1() {
  uint x_1 = 42u;
  const uint x_2 = x_1;
  x_1 = x_2;
  x_1 = x_2;
  return;
}

void main() {
  main_1();
  return;
}
