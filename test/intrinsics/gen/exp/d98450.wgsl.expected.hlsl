void exp_d98450() {
  float3 res = exp(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  exp_d98450();
  return;
}

void fragment_main() {
  exp_d98450();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp_d98450();
  return;
}

