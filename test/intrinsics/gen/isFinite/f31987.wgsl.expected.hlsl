void isFinite_f31987() {
  vector<bool, 4> res = isfinite(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  isFinite_f31987();
  return;
}

void fragment_main() {
  isFinite_f31987();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isFinite_f31987();
  return;
}

