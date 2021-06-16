SKIP: FAILED



Validation Failure:
void fma_26a7a9() {
  float2 res = fma(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  fma_26a7a9();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  fma_26a7a9();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fma_26a7a9();
  return;
}

tint_9GH744:2:16: error: no matching function for call to 'fma'
  float2 res = fma(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 2>' to 'vector<double, 2>' for 1st argument


tint_9GH744:2:16: error: no matching function for call to 'fma'
  float2 res = fma(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 2>' to 'vector<double, 2>' for 1st argument


tint_9GH744:2:16: error: no matching function for call to 'fma'
  float2 res = fma(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 2>' to 'vector<double, 2>' for 1st argument

