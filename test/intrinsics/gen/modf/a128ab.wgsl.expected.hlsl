groupshared float2 arg_1;

void modf_a128ab() {
  float2 res = modf(float2(0.0f, 0.0f), arg_1);
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_a128ab();
  return;
}
