void length_becebf() {
  float res = length(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  length_becebf();
  return;
}

void fragment_main() {
  length_becebf();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  length_becebf();
  return;
}

