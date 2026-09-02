
RWByteAddressBuffer prevent_dce : register(u0);
TextureCubeArray arg_0 : register(t0, space1);
SamplerState arg_1 : register(s1, space1);
float textureSample_7fd8cb() {
  float v = float(1u);
  float res = arg_0.Sample(arg_1, (1.0f).xxxx).x;
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(textureSample_7fd8cb()));
}

