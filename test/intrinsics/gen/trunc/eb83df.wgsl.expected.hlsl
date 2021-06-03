void trunc_eb83df() {
  float res = trunc(1.0f);
}

void vertex_main() {
  trunc_eb83df();
  return;
}

void fragment_main() {
  trunc_eb83df();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  trunc_eb83df();
  return;
}

