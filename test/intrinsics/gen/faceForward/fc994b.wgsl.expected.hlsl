void faceForward_fc994b() {
  float res = faceforward(1.0f, 1.0f, 1.0f);
}

void vertex_main() {
  faceForward_fc994b();
  return;
}

void fragment_main() {
  faceForward_fc994b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  faceForward_fc994b();
  return;
}

