SKIP: FAILED

void main_1() {
  uint x_1 = 42u;
  uint x_2 = x_1;
  x_1 = x_1;
  x_1 = x_2;
}

void main() {
  main_1();
}

