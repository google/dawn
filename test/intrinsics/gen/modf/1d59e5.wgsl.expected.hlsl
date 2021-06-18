groupshared float4 arg_1;

void modf_1d59e5() {
  float4 res = modf(float4(0.0f, 0.0f, 0.0f, 0.0f), arg_1);
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_1d59e5();
  return;
}
