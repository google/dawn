void main_1() {
  int2 x_200 = int2(0, 0);
  if (true) {
    x_200 = int2(0, 0);
    x_200.x = 0;
  } else {
    return;
  }
  const int2 x_201 = x_200;
  return;
}

void main() {
  main_1();
  return;
}
