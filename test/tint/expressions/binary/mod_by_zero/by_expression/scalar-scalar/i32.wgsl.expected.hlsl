int value_or_one_if_zero_int(int value) {
  return value == 0 ? 1 : value;
}

[numthreads(1, 1, 1)]
void f() {
  int a = 1;
  int b = 0;
  const int r = (a % value_or_one_if_zero_int((b + b)));
  return;
}
