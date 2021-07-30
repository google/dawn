uint x_50() {
  return 42u;
}

void x_100_1() {
  uint x_10 = 0u;
  const uint x_1 = x_50();
  x_10 = x_1;
  x_10 = x_1;
  return;
}

void x_100() {
  x_100_1();
  return;
}
