void isFinite_8a23ad() {
  vector<bool, 3> res = isfinite(float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  isFinite_8a23ad();
  return;
}

void fragment_main() {
  isFinite_8a23ad();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isFinite_8a23ad();
  return;
}

