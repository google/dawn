[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void foo() {
  int tint_symbol[2] = (int[2])0;
  int implict[2] = (int[2])0;
  implict = tint_symbol;
}
