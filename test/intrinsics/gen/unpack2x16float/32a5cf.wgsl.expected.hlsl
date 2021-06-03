void unpack2x16float_32a5cf() {
  uint tint_tmp = 1u;
float2 res = f16tof32(uint2(tint_tmp & 0xffff, tint_tmp >> 16));
}

void vertex_main() {
  unpack2x16float_32a5cf();
  return;
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

