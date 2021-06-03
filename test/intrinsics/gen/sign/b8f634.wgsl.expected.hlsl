void sign_b8f634() {
  float4 res = sign(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  sign_b8f634();
  return;
}

void fragment_main() {
  sign_b8f634();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sign_b8f634();
  return;
}

