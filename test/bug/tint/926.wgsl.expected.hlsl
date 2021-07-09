RWByteAddressBuffer drawOut : register(u5, space0);
static uint cubeVerts = 0u;

struct tint_symbol_1 {
  uint3 global_id : SV_DispatchThreadID;
};

[numthreads(1, 1, 1)]
void computeMain(tint_symbol_1 tint_symbol) {
  const uint3 global_id = tint_symbol.global_id;
  uint atomic_result = 0u;
  drawOut.InterlockedAdd(0u, cubeVerts, atomic_result);
  const uint firstVertex = atomic_result;
  return;
}
