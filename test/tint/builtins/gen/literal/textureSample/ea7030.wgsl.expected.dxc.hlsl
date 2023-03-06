TextureCube arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
RWByteAddressBuffer prevent_dce : register(u0, space2);

void textureSample_ea7030() {
  float res = arg_0.Sample(arg_1, (1.0f).xxx).x;
  prevent_dce.Store(0u, asuint(res));
}

void fragment_main() {
  textureSample_ea7030();
  return;
}
