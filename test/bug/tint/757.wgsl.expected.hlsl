bug/tint/757.wgsl:3:5 warning: use of deprecated language feature: [[offset]] has been replaced with [[size]] and [[align]]
  [[offset(0)]] level : i32;
    ^^^^^^

bug/tint/757.wgsl:10:5 warning: use of deprecated language feature: [[offset]] has been replaced with [[size]] and [[align]]
  [[offset(0)]] values : [[stride(4)]] array<f32>;
    ^^^^^^

struct Constants {
  int level;
};
struct tint_symbol_1 {
  uint3 GlobalInvocationID : SV_DispatchThreadID;
};

RWByteAddressBuffer result : register(u3, space0);

Texture2DArray<float4> myTexture : register(t1, space0);

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  const uint3 GlobalInvocationID = tint_symbol.GlobalInvocationID;
  uint flatIndex = ((((2u * 2u) * GlobalInvocationID.z) + (2u * GlobalInvocationID.y)) + GlobalInvocationID.x);
  flatIndex = (flatIndex * 1u);
  float4 texel = myTexture.Load(int4(GlobalInvocationID.xy, 0, 0), 0);
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

