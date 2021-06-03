void ceil_678655() {
  float res = ceil(1.0f);
}

void vertex_main() {
  ceil_678655();
  return;
}

void fragment_main() {
  ceil_678655();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ceil_678655();
  return;
}

