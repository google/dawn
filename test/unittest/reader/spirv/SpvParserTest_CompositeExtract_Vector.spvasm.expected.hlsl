void main_1() {
  const float x_1 = float2(50.0f, 60.0f).y;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
