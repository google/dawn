SKIP: FAILED



Validation Failure:
Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_7e9ffd() {
  float res = arg_0.Sample(arg_1, float3(float(1)));
}

void fragment_main() {
  textureSample_7e9ffd();
  return;
}


tint_RyRxHD:5:42: error: too few elements in vector initialization (expected 3 elements, have 1)
  float res = arg_0.Sample(arg_1, float3(float(1)));
                                         ^
tint_RyRxHD:5:9: warning: implicit truncation of vector type [-Wconversion]
  float res = arg_0.Sample(arg_1, float3(float(1)));
        ^

