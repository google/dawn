void faceForward_e6908b() {
  float2 res = faceforward(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  faceForward_e6908b();
  return;
}

void fragment_main() {
  faceForward_e6908b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  faceForward_e6908b();
  return;
}

