void unpack2x16float_32a5cf() {
  uint tint_tmp = 1u;
  float2 res = f16tof32(uint2(tint_tmp & 0xffff, tint_tmp >> 16));
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  unpack2x16float_32a5cf();
  const tint_symbol tint_symbol_1 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_1;
}

void fragment_main() {
  unpack2x16float_32a5cf();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  unpack2x16float_32a5cf();
  return;
}
