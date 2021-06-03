void length_602a17() {
  float res = length(1.0f);
}

void vertex_main() {
  length_602a17();
  return;
}

void fragment_main() {
  length_602a17();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  length_602a17();
  return;
}

