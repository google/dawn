SKIP: FAILED



Validation Failure:
void fma_c10ba3() {
  float res = fma(1.0f, 1.0f, 1.0f);
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  fma_c10ba3();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
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

tint_tqaLcU:2:15: error: no matching function for call to 'fma'
  float res = fma(1.0f, 1.0f, 1.0f);
              ^~~
note: candidate function not viable: no known conversion from 'float' to 'double' for 1st argument


tint_tqaLcU:2:15: error: no matching function for call to 'fma'
  float res = fma(1.0f, 1.0f, 1.0f);
              ^~~
note: candidate function not viable: no known conversion from 'float' to 'double' for 1st argument


tint_tqaLcU:2:15: error: no matching function for call to 'fma'
  float res = fma(1.0f, 1.0f, 1.0f);
              ^~~
note: candidate function not viable: no known conversion from 'float' to 'double' for 1st argument

