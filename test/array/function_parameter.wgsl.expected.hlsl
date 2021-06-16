struct tint_array_wrapper {
  float arr[4];
};

float f1(tint_array_wrapper a) {
  return a.arr[3];
}

struct tint_array_wrapper_1 {
  tint_array_wrapper arr[3];
};

float f2(tint_array_wrapper_1 a) {
  return a.arr[2].arr[3];
}

struct tint_array_wrapper_2 {
  tint_array_wrapper_1 arr[2];
};

float f3(tint_array_wrapper_2 a) {
  return a.arr[1].arr[2].arr[3];
}

[numthreads(1, 1, 1)]
void main() {
  const tint_array_wrapper a1 = {{0.0f, 0.0f, 0.0f, 0.0f}};
  const tint_array_wrapper_1 a2 = {{{{0.0f, 0.0f, 0.0f, 0.0f}}, {{0.0f, 0.0f, 0.0f, 0.0f}}, {{0.0f, 0.0f, 0.0f, 0.0f}}}};
  const tint_array_wrapper_2 a3 = {{{{{{0.0f, 0.0f, 0.0f, 0.0f}}, {{0.0f, 0.0f, 0.0f, 0.0f}}, {{0.0f, 0.0f, 0.0f, 0.0f}}}}, {{{{0.0f, 0.0f, 0.0f, 0.0f}}, {{0.0f, 0.0f, 0.0f, 0.0f}}, {{0.0f, 0.0f, 0.0f, 0.0f}}}}}};
  const float v1 = f1(a1);
  const float v2 = f2(a2);
  const float v3 = f3(a3);
  return;
}
