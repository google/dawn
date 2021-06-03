SKIP: FAILED



Validation Failure:
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_02aa9b() {
  float4 res = arg_0.Sample(arg_1, float3(float(1)), int2(0, 0));
}

void fragment_main() {
  textureSample_02aa9b();
  return;
}


tint_O72GUO:5:43: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Sample(arg_1, float3(float(1)), int2(0, 0));
                                          ^

