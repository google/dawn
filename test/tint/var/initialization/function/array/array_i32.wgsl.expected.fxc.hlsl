[numthreads(1, 1, 1)]
void main() {
  int zero[2][3] = (int[2][3])0;
  const int tint_symbol[3] = {1, 2, 3};
  const int tint_symbol_1[3] = {4, 5, 6};
  int init[2][3] = {tint_symbol, tint_symbol_1};
  return;
}
