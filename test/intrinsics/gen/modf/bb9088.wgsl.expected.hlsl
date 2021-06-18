groupshared float3 arg_1;

void modf_bb9088() {
  float3 res = modf(float3(0.0f, 0.0f, 0.0f), arg_1);
}

[numthreads(1, 1, 1)]
void compute_main() {
  modf_bb9088();
  return;
}
