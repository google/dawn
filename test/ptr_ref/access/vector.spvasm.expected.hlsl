void main_1() {
  float3 v = float3(0.0f, 0.0f, 0.0f);
  v = float3(1.0f, 2.0f, 3.0f);
  v.y = 5.0f;
  return;
}

[numthreads(1, 1, 1)]
void main() {
  main_1();
  return;
}
