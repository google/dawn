void sign_d065d8() {
  float2 res = sign(float2(0.0f, 0.0f));
}

void vertex_main() {
  sign_d065d8();
  return;
}

void fragment_main() {
  sign_d065d8();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  sign_d065d8();
  return;
}

