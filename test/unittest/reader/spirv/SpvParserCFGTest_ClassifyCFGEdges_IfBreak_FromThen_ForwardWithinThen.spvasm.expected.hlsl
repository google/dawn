static uint var_1 = 0u;

void main_1() {
  bool guard10 = true;
  if (false) {
    if (true) {
      guard10 = false;
    }
    if (guard10) {
      guard10 = false;
    }
  }
  return;
}

void main() {
  main_1();
  return;
}
