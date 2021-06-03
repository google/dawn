SKIP: FAILED



Validation Failure:
Texture2DArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_6717ca() {
  float4 res = arg_0.Sample(arg_1, float3(float(1)));
}

void fragment_main() {
  textureSample_6717ca();
  return;
}


tint_0ZWvLf:5:43: error: too few elements in vector initialization (expected 3 elements, have 1)
  float4 res = arg_0.Sample(arg_1, float3(float(1)));
                                          ^

