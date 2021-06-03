SKIP: FAILED



Validation Failure:
Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSampleLevel_1bf73e() {
  float res = arg_0.SampleLevel(arg_1, float3(float(1)), 1);
}

void vertex_main() {
  textureSampleLevel_1bf73e();
  return;
}

void fragment_main() {
  textureSampleLevel_1bf73e();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureSampleLevel_1bf73e();
  return;
}


tint_U83SMG:5:47: error: too few elements in vector initialization (expected 3 elements, have 1)
  float res = arg_0.SampleLevel(arg_1, float3(float(1)), 1);
                                              ^
tint_U83SMG:5:9: warning: implicit truncation of vector type [-Wconversion]
  float res = arg_0.SampleLevel(arg_1, float3(float(1)), 1);
        ^


tint_U83SMG:5:47: error: too few elements in vector initialization (expected 3 elements, have 1)
  float res = arg_0.SampleLevel(arg_1, float3(float(1)), 1);
                                              ^
tint_U83SMG:5:9: warning: implicit truncation of vector type [-Wconversion]
  float res = arg_0.SampleLevel(arg_1, float3(float(1)), 1);
        ^


tint_U83SMG:5:47: error: too few elements in vector initialization (expected 3 elements, have 1)
  float res = arg_0.SampleLevel(arg_1, float3(float(1)), 1);
                                              ^
tint_U83SMG:5:9: warning: implicit truncation of vector type [-Wconversion]
  float res = arg_0.SampleLevel(arg_1, float3(float(1)), 1);
        ^

