
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSample_60bf45() {
  Texture2DArray v = arg_0;
  SamplerState v_1 = arg_1;
  float res = v.Sample(v_1, float3((1.0f).xx, float(1)), (1).xx);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSample_60bf45()));
}

