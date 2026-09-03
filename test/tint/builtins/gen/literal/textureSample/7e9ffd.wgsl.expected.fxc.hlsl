
RWByteAddressBuffer prevent_dce : register(u0);
Texture2DArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSample_7e9ffd() {
  float res = arg_0.Sample(arg_1, (1.0f).xxx).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSample_7e9ffd()));
}

