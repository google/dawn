groupshared float arg_1;

void modf_e38ae6() {
  float res = modf(1.0f, arg_1);
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_e38ae6();
  return;
}
