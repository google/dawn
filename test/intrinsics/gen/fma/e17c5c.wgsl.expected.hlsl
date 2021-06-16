SKIP: FAILED



Validation Failure:
void fma_e17c5c() {
  float3 res = fma(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  fma_e17c5c();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
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

tint_NJUQps:2:16: error: no matching function for call to 'fma'
  float3 res = fma(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 3>' to 'vector<double, 3>' for 1st argument


tint_NJUQps:2:16: error: no matching function for call to 'fma'
  float3 res = fma(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 3>' to 'vector<double, 3>' for 1st argument


tint_NJUQps:2:16: error: no matching function for call to 'fma'
  float3 res = fma(float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 0.0f, 0.0f));
               ^~~
note: candidate function not viable: no known conversion from 'vector<float, 3>' to 'vector<double, 3>' for 1st argument

