void ldexp_f54ff2() {
  float res = ldexp(1.0f, 1u);
}

void vertex_main() {
  ldexp_f54ff2();
  return;
}

void fragment_main() {
  ldexp_f54ff2();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_f54ff2();
  return;
}

