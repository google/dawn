cbuffer cbuffer_constants : register(b0, space0) {
  uint4 constants[1];
};
Texture2DArray<float4> myTexture : register(t1, space0);

RWByteAddressBuffer result : register(u3, space0);

struct tint_symbol_1 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  const uint3 GlobalInvocationID = tint_symbol.GlobalInvocationID;
  uint flatIndex = ((((2u * 2u) * GlobalInvocationID.z) + (2u * GlobalInvocationID.y)) + GlobalInvocationID.x);
  flatIndex = (flatIndex * 1u);
  float4 texel = myTexture.Load(int4(GlobalInvocationID.xy, 0, 0));
  {
    uint i = 0u;
    while (true) {
      if (!((i < 1u))) {
        break;
      }
      result.Store((4u * (flatIndex + i)), asuint(texel.r));
      {
        i = (i + 1u);
      }
    }
  }
  return;
}
