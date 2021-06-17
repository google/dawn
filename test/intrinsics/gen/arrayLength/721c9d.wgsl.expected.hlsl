intrinsics/gen/arrayLength/721c9d.wgsl:33:18 warning: use of deprecated intrinsic
  var res: u32 = arrayLength(sb.arg_0);
                 ^^^^^^^^^^^

ByteAddressBuffer sb : register(t0, space0);

void arrayLength_721c9d() {
  uint tint_symbol_2 = 0u;
  sb.GetDimensions(tint_symbol_2);
  const uint tint_symbol_3 = ((tint_symbol_2 - 0u) / 4u);
  uint res = tint_symbol_3;
}

struct tint_symbol {
  float4 value : SV_Position;
};

tint_symbol vertex_main() {
  arrayLength_721c9d();
  const tint_symbol tint_symbol_4 = {float4(0.0f, 0.0f, 0.0f, 0.0f)};
  return tint_symbol_4;
}

void fragment_main() {
  arrayLength_721c9d();
  return;
}

[numthreads(1, 1, 1)]
void compute_main() {
  arrayLength_721c9d();
  return;
}
