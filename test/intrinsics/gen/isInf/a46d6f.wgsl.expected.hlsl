void isInf_a46d6f() {
  vector<bool, 2> res = isinf(float2(0.0f, 0.0f));
}

void vertex_main() {
  isInf_a46d6f();
  return;
}

void fragment_main() {
  isInf_a46d6f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isInf_a46d6f();
  return;
}

