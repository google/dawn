void trunc_f370d3() {
  float2 res = trunc(float2(0.0f, 0.0f));
}

void vertex_main() {
  trunc_f370d3();
  return;
}

void fragment_main() {
  trunc_f370d3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  trunc_f370d3();
  return;
}

