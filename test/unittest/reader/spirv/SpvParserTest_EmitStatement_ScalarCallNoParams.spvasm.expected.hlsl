uint x_50() {
  return 42u;
}

void x_100_1() {
  const uint x_1 = x_50();
  return;
}

void x_100() {
  x_100_1();
  return;
}
