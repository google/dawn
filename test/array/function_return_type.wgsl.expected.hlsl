struct tint_array_wrapper {
  float arr[4];
};

tint_array_wrapper f1() {
  const tint_array_wrapper tint_symbol = {{0.0f, 0.0f, 0.0f, 0.0f}};
  return tint_symbol;
}

struct tint_array_wrapper_1 {
  tint_array_wrapper arr[3];
};

tint_array_wrapper_1 f2() {
  const tint_array_wrapper_1 tint_symbol_1 = {{f1(), f1(), f1()}};
  return tint_symbol_1;
}

struct tint_array_wrapper_2 {
  tint_array_wrapper_1 arr[2];
};

tint_array_wrapper_2 f3() {
  const tint_array_wrapper_2 tint_symbol_2 = {{f2(), f2()}};
  return tint_symbol_2;
}

[numthreads(1, 1, 1)]
void main() {
  const tint_array_wrapper a1 = f1();
  const tint_array_wrapper_1 a2 = f2();
  const tint_array_wrapper_2 a3 = f3();
  return;
}
