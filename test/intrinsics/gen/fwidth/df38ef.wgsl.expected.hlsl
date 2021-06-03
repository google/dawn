void fwidth_df38ef() {
  float res = fwidth(1.0f);
}

void vertex_main() {
  fwidth_df38ef();
  return;
}

void fragment_main() {
  fwidth_df38ef();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fwidth_df38ef();
  return;
}

