SKIP: FAILED



Validation Failure:
TextureCubeArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);

void textureSample_c2f4e8() {
  float res = arg_0.Sample(arg_1, float4(float(1)));
}

void fragment_main() {
  textureSample_c2f4e8();
  return;
}


tint_YC11Cc:5:42: error: too few elements in vector initialization (expected 4 elements, have 1)
  float res = arg_0.Sample(arg_1, float4(float(1)));
                                         ^
tint_YC11Cc:5:9: warning: implicit truncation of vector type [-Wconversion]
  float res = arg_0.Sample(arg_1, float4(float(1)));
        ^

