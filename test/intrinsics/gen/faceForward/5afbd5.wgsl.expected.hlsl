void faceForward_5afbd5() {
  float3 res = faceforward(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  faceForward_5afbd5();
  return;
}

void fragment_main() {
  faceForward_5afbd5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  faceForward_5afbd5();
  return;
}

