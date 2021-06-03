SKIP: FAILED



Validation Failure:
TextureCubeArray<float4> arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_4dd1bf() {
  float4 res = arg_0.Sample(arg_1, float4(float(1)));
}

void fragment_main() {
  textureSample_4dd1bf();
  return;
}


tint_KfNSDx:5:43: error: too few elements in vector initialization (expected 4 elements, have 1)
  float4 res = arg_0.Sample(arg_1, float4(float(1)));
                                          ^

