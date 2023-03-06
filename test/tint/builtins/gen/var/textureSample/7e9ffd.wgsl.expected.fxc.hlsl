Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSample_7e9ffd() {
  float2 arg_2 = (1.0f).xx;
  int arg_3 = 1;
  float res = arg_0.Sample(arg_1, float3(arg_2, float(arg_3))).x;
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  textureSample_7e9ffd();
  return;
}
