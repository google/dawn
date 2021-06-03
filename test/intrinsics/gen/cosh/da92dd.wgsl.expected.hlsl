void cosh_da92dd() {
  float res = cosh(1.0f);
}

void vertex_main() {
  cosh_da92dd();
  return;
}

void fragment_main() {
  cosh_da92dd();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  cosh_da92dd();
  return;
}

