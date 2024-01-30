void set_vector_element(inout int3 vec, int idx, int val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void deref() {
  int3 a = int3(0, 0, 0);
  int tint_symbol_1 = 0;
  set_vector_element(a, tint_symbol_1, (a[tint_symbol_1] + 42));
}

void no_deref() {
  int3 a = int3(0, 0, 0);
  int tint_symbol_3 = 0;
  set_vector_element(a, tint_symbol_3, (a[tint_symbol_3] + 42));
}

void deref_inc() {
  int3 a = int3(0, 0, 0);
  int tint_symbol_5 = 0;
  set_vector_element(a, tint_symbol_5, (a[tint_symbol_5] + 1));
}

void no_deref_inc() {
  int3 a = int3(0, 0, 0);
  int tint_symbol_7 = 0;
  set_vector_element(a, tint_symbol_7, (a[tint_symbol_7] + 1));
}

[numthreads(1, 1, 1)]
void main() {
  deref();
  no_deref();
  deref_inc();
  no_deref_inc();
  return;
}
