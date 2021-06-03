void reflect_feae90() {
  float res = reflect(1.0f, 1.0f);
}

void vertex_main() {
  reflect_feae90();
  return;
}

void fragment_main() {
  reflect_feae90();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  reflect_feae90();
  return;
}

