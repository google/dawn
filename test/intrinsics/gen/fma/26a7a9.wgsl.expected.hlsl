SKIP: FAILED



Validation Failure:
void fma_26a7a9() {
  float2 res = fma(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
}

void vertex_main() {
  fma_26a7a9();
  return;
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


tint_EKmo0D:2:16: error: no matching function for call to 'fma'
  float2 res = fma(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 2>' to 'vector<double, 2>' for 1st argument


tint_EKmo0D:2:16: error: no matching function for call to 'fma'
  float2 res = fma(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 2>' to 'vector<double, 2>' for 1st argument


tint_EKmo0D:2:16: error: no matching function for call to 'fma'
  float2 res = fma(float2(0.0f, 0.0f), float2(0.0f, 0.0f), float2(0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 2>' to 'vector<double, 2>' for 1st argument

