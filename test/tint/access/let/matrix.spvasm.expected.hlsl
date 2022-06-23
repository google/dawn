void main_1() {
  const float x_24 = float3(4.0f, 5.0f, 6.0f).y;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
