void main_1() {
  bool x_101_phi = false;
  const bool x_11 = (true & true);
  const bool x_12 = !(x_11);
  x_101_phi = x_11;
  if (true) {
    x_101_phi = x_12;
  }
  const bool x_101 = x_101_phi;
  return;
}

void main() {
  main_1();
  return;
}
