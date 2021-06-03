SKIP: FAILED



Validation Failure:
void fma_c10ba3() {
  float res = fma(1.0f, 1.0f, 1.0f);
}

void vertex_main() {
  fma_c10ba3();
  return;
}

void fragment_main() {
  fma_c10ba3();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fma_c10ba3();
  return;
}


tint_6WuQ1K:2:15: error: no matching function for call to 'fma'
  float res = fma(1.0f, 1.0f, 1.0f);
              ^~~
note: candidate function not viable: no known conversion from 'float' to 'double' for 1st argument


tint_6WuQ1K:2:15: error: no matching function for call to 'fma'
  float res = fma(1.0f, 1.0f, 1.0f);
              ^~~
note: candidate function not viable: no known conversion from 'float' to 'double' for 1st argument


tint_6WuQ1K:2:15: error: no matching function for call to 'fma'
  float res = fma(1.0f, 1.0f, 1.0f);
              ^~~
note: candidate function not viable: no known conversion from 'float' to 'double' for 1st argument

