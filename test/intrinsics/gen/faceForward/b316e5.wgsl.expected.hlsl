void faceForward_b316e5() {
  float4 res = faceforward(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  faceForward_b316e5();
  return;
}

void fragment_main() {
  faceForward_b316e5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  faceForward_b316e5();
  return;
}

