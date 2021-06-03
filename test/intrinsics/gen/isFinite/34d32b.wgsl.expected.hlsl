void isFinite_34d32b() {
  vector<bool, 2> res = isfinite(float2(0.0f, 0.0f));
}

void vertex_main() {
  isFinite_34d32b();
  return;
}

void fragment_main() {
  isFinite_34d32b();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isFinite_34d32b();
  return;
}

