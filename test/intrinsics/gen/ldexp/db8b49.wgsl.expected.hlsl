void ldexp_db8b49() {
  float res = ldexp(1.0f, 1);
}

void vertex_main() {
  ldexp_db8b49();
  return;
}

void fragment_main() {
  ldexp_db8b49();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  ldexp_db8b49();
  return;
}

