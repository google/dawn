Texture2D arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSample_38bbb9() {
  float2 arg_2 = (1.0f).xx;
  float res = arg_0.Sample(arg_1, arg_2).x;
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  textureSample_38bbb9();
  return;
}
