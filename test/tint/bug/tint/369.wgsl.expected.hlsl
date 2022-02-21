[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

ByteAddressBuffer SSBO : register(t0, space0);
cbuffer cbuffer_UBO : register(b0, space0) {
  uint4 UBO[1];
};
