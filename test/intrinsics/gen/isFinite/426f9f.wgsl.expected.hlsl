void isFinite_426f9f() {
  bool res = isfinite(1.0f);
}

void vertex_main() {
  isFinite_426f9f();
  return;
}

void fragment_main() {
  isFinite_426f9f();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  isFinite_426f9f();
  return;
}

