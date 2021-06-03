void tanh_c15fdb() {
  float res = tanh(1.0f);
}

void vertex_main() {
  tanh_c15fdb();
  return;
}

void fragment_main() {
  tanh_c15fdb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  tanh_c15fdb();
  return;
}

