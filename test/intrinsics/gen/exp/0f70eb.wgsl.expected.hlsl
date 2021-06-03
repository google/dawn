void exp_0f70eb() {
  float4 res = exp(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  exp_0f70eb();
  return;
}

void fragment_main() {
  exp_0f70eb();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  exp_0f70eb();
  return;
}

