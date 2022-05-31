RWByteAddressBuffer drawOut : register(u5, space0);
static uint cubeVerts = 0u;

struct tint_symbol_1 {
  uint3 global_id : SV_DispatchThreadID;
};

uint tint_atomicAdd(RWByteAddressBuffer buffer, uint offset, uint value) {
  uint original_value = 0;
  buffer.InterlockedAdd(offset, value, original_value);
  return original_value;
}


void computeMain_inner(uint3 global_id) {
  const uint firstVertex = tint_atomicAdd(drawOut, 0u, cubeVerts);
}

[numthreads(1, 1, 1)]
void computeMain(tint_symbol_1 tint_symbol) {
  computeMain_inner(tint_symbol.global_id);
  return;
}
