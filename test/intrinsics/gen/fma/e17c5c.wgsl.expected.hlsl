SKIP: FAILED



Validation Failure:
void fma_e17c5c() {
  float3 res = fma(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

void vertex_main() {
  fma_e17c5c();
  return;
}

void fragment_main() {
  fma_e17c5c();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  fma_e17c5c();
  return;
}


tint_5UHLns:2:16: error: no matching function for call to 'fma'
  float3 res = fma(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 3>' to 'vector<double, 3>' for 1st argument


tint_5UHLns:2:16: error: no matching function for call to 'fma'
  float3 res = fma(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 3>' to 'vector<double, 3>' for 1st argument


tint_5UHLns:2:16: error: no matching function for call to 'fma'
  float3 res = fma(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 3>' to 'vector<double, 3>' for 1st argument

