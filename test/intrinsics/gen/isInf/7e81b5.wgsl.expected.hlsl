void isInf_7e81b5() {
  vector<bool, 4> res = isinf(float4(0.0f, 0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  isInf_7e81b5();
  return;
}

void fragment_main() {
  isInf_7e81b5();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isInf_7e81b5();
  return;
}

