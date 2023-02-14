struct Inner {
  int scalar_i32;
  float scalar_f32;
};

RWByteAddressBuffer sb : register(u0, space0);

void tint_symbol_12(RWByteAddressBuffer buffer, uint offset, float2x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
}

void tint_symbol_13(RWByteAddressBuffer buffer, uint offset, float2x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
}

void tint_symbol_14(RWByteAddressBuffer buffer, uint offset, float2x4 value) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
}

void tint_symbol_15(RWByteAddressBuffer buffer, uint offset, float3x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
}

void tint_symbol_16(RWByteAddressBuffer buffer, uint offset, float3x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
  buffer.Store3((offset + 32u), asuint(value[2u]));
}

void tint_symbol_17(RWByteAddressBuffer buffer, uint offset, float3x4 value) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
  buffer.Store4((offset + 32u), asuint(value[2u]));
}

void tint_symbol_18(RWByteAddressBuffer buffer, uint offset, float4x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
  buffer.Store2((offset + 24u), asuint(value[3u]));
}

void tint_symbol_19(RWByteAddressBuffer buffer, uint offset, float4x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
  buffer.Store3((offset + 32u), asuint(value[2u]));
  buffer.Store3((offset + 48u), asuint(value[3u]));
}

void tint_symbol_20(RWByteAddressBuffer buffer, uint offset, float4x4 value) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
  buffer.Store4((offset + 32u), asuint(value[2u]));
  buffer.Store4((offset + 48u), asuint(value[3u]));
}

void tint_symbol_21(RWByteAddressBuffer buffer, uint offset, float3 value[2]) {
  float3 array_1[2] = value;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      buffer.Store3((offset + (i * 16u)), asuint(array_1[i]));
    }
  }
}

void tint_symbol_22(RWByteAddressBuffer buffer, uint offset, Inner value) {
  buffer.Store((offset + 0u), asuint(value.scalar_i32));
  buffer.Store((offset + 4u), asuint(value.scalar_f32));
}

void tint_symbol_23(RWByteAddressBuffer buffer, uint offset, Inner value[4]) {
  Inner array_2[4] = value;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      tint_symbol_22(buffer, (offset + (i_1 * 8u)), array_2[i_1]);
    }
  }
}

[numthreads(1, 1, 1)]
void main() {
  sb.Store(0u, asuint(0.0f));
  sb.Store(4u, asuint(0));
  sb.Store(8u, asuint(0u));
  sb.Store2(16u, asuint((0.0f).xx));
  sb.Store2(24u, asuint((0).xx));
  sb.Store2(32u, asuint((0u).xx));
  sb.Store3(48u, asuint((0.0f).xxx));
  sb.Store3(64u, asuint((0).xxx));
  sb.Store3(80u, asuint((0u).xxx));
  sb.Store4(96u, asuint((0.0f).xxxx));
  sb.Store4(112u, asuint((0).xxxx));
  sb.Store4(128u, asuint((0u).xxxx));
  tint_symbol_12(sb, 144u, float2x2((0.0f).xx, (0.0f).xx));
  tint_symbol_13(sb, 160u, float2x3((0.0f).xxx, (0.0f).xxx));
  tint_symbol_14(sb, 192u, float2x4((0.0f).xxxx, (0.0f).xxxx));
  tint_symbol_15(sb, 224u, float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  tint_symbol_16(sb, 256u, float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  tint_symbol_17(sb, 304u, float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  tint_symbol_18(sb, 352u, float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  tint_symbol_19(sb, 384u, float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  tint_symbol_20(sb, 448u, float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  const float3 tint_symbol_24[2] = (float3[2])0;
  tint_symbol_21(sb, 512u, tint_symbol_24);
  const Inner tint_symbol_25 = (Inner)0;
  tint_symbol_22(sb, 544u, tint_symbol_25);
  const Inner tint_symbol_26[4] = (Inner[4])0;
  tint_symbol_23(sb, 552u, tint_symbol_26);
  return;
}
