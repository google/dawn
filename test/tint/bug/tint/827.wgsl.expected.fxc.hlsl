Texture2D tex : register(t0);
RWByteAddressBuffer result : register(u1);

struct tint_symbol_1 {
  uint3 GlobalInvocationId : SV_DispatchThreadID;
};

void main_inner(uint3 GlobalInvocationId) {
  result.Store((4u * ((GlobalInvocationId.y * 128u) + GlobalInvocationId.x)), asuint(tex.Load(int3(int(GlobalInvocationId.x), int(GlobalInvocationId.y), 0)).x));
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.GlobalInvocationId);
  return;
}
