void main_1() {
  const float2 distance_1 = (2.0f).xx;
  const float x_10 = distance(distance_1, (2.0f).xx);
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
