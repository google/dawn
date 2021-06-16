SKIP: FAILED



Validation Failure:
void fma_6a3283() {
  float4 res = fma(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  fma_6a3283();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}
void fragment_main() {
  fma_6a3283();
  return;
}
[numthreads(1, 1, 1)]
void compute_main() {
  fma_6a3283();
  return;
}

tint_JBvC4T:2:16: error: no matching function for call to 'fma'
  float4 res = fma(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 4>' to 'vector<double, 4>' for 1st argument


tint_JBvC4T:2:16: error: no matching function for call to 'fma'
  float4 res = fma(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 4>' to 'vector<double, 4>' for 1st argument


tint_JBvC4T:2:16: error: no matching function for call to 'fma'
  float4 res = fma(float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f), float4(0.0f, 0.0f, 0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 4>' to 'vector<double, 4>' for 1st argument

