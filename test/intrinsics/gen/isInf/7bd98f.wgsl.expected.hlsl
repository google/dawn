void isInf_7bd98f() {
  bool res = isinf(1.0f);
}

void vertex_main() {
  isInf_7bd98f();
  return;
}

void fragment_main() {
  isInf_7bd98f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isInf_7bd98f();
  return;
}

