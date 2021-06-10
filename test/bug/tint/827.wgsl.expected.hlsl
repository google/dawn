bug/tint/827.wgsl:8:26 warning: use of deprecated language feature: declare access with var<storage, read_write> instead of using [[access]] decoration
[[group(0), binding(1)]] var<storage> result : [[access(read_write)]] Result;
                         ^^^

struct tint_symbol_1 {
  uint3 GlobalInvocationId : SV_DispatchThreadID;
};

static const uint width = 128u;
RWByteAddressBuffer result : register(u1, space0);

Texture2D tex : register(t0, space0);

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  const uint3 GlobalInvocationId = tint_symbol.GlobalInvocationId;
  result.Store((4u * ((GlobalInvocationId.y * width) + GlobalInvocationId.x)), asuint(tex.Load(int3(int(GlobalInvocationId.x), int(GlobalInvocationId.y), 0), 0)));
  return;
}

