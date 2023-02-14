struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};

RWByteAddressBuffer sb : register(u0, space0);

void tint_symbol_16(RWByteAddressBuffer buffer, uint offset, float2x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
}

void tint_symbol_17(RWByteAddressBuffer buffer, uint offset, float2x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
}

void tint_symbol_18(RWByteAddressBuffer buffer, uint offset, float2x4 value) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
}

void tint_symbol_19(RWByteAddressBuffer buffer, uint offset, float3x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
}

void tint_symbol_20(RWByteAddressBuffer buffer, uint offset, float3x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
  buffer.Store3((offset + 32u), asuint(value[2u]));
}

void tint_symbol_21(RWByteAddressBuffer buffer, uint offset, float3x4 value) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
  buffer.Store4((offset + 32u), asuint(value[2u]));
}

void tint_symbol_22(RWByteAddressBuffer buffer, uint offset, float4x2 value) {
  buffer.Store2((offset + 0u), asuint(value[0u]));
  buffer.Store2((offset + 8u), asuint(value[1u]));
  buffer.Store2((offset + 16u), asuint(value[2u]));
  buffer.Store2((offset + 24u), asuint(value[3u]));
}

void tint_symbol_23(RWByteAddressBuffer buffer, uint offset, float4x3 value) {
  buffer.Store3((offset + 0u), asuint(value[0u]));
  buffer.Store3((offset + 16u), asuint(value[1u]));
  buffer.Store3((offset + 32u), asuint(value[2u]));
  buffer.Store3((offset + 48u), asuint(value[3u]));
}

void tint_symbol_24(RWByteAddressBuffer buffer, uint offset, float4x4 value) {
  buffer.Store4((offset + 0u), asuint(value[0u]));
  buffer.Store4((offset + 16u), asuint(value[1u]));
  buffer.Store4((offset + 32u), asuint(value[2u]));
  buffer.Store4((offset + 48u), asuint(value[3u]));
}

void tint_symbol_25(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 2, 2> value) {
  buffer.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
}

void tint_symbol_26(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 2, 3> value) {
  buffer.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
}

void tint_symbol_27(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 2, 4> value) {
  buffer.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
}

void tint_symbol_28(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 3, 2> value) {
  buffer.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
  buffer.Store<vector<float16_t, 2> >((offset + 8u), value[2u]);
}

void tint_symbol_29(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 3, 3> value) {
  buffer.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
  buffer.Store<vector<float16_t, 3> >((offset + 16u), value[2u]);
}

void tint_symbol_30(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 3, 4> value) {
  buffer.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
  buffer.Store<vector<float16_t, 4> >((offset + 16u), value[2u]);
}

void tint_symbol_31(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 4, 2> value) {
  buffer.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
  buffer.Store<vector<float16_t, 2> >((offset + 8u), value[2u]);
  buffer.Store<vector<float16_t, 2> >((offset + 12u), value[3u]);
}

void tint_symbol_32(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 4, 3> value) {
  buffer.Store<vector<float16_t, 3> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 3> >((offset + 8u), value[1u]);
  buffer.Store<vector<float16_t, 3> >((offset + 16u), value[2u]);
  buffer.Store<vector<float16_t, 3> >((offset + 24u), value[3u]);
}

void tint_symbol_33(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 4, 4> value) {
  buffer.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  buffer.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
  buffer.Store<vector<float16_t, 4> >((offset + 16u), value[2u]);
  buffer.Store<vector<float16_t, 4> >((offset + 24u), value[3u]);
}

void tint_symbol_34(RWByteAddressBuffer buffer, uint offset, float3 value[2]) {
  float3 array_1[2] = value;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      buffer.Store3((offset + (i * 16u)), asuint(array_1[i]));
    }
  }
}

void tint_symbol_35(RWByteAddressBuffer buffer, uint offset, matrix<float16_t, 4, 2> value[2]) {
  matrix<float16_t, 4, 2> array_2[2] = value;
  {
    for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      tint_symbol_31(buffer, (offset + (i_1 * 16u)), array_2[i_1]);
    }
  }
}

void tint_symbol_36(RWByteAddressBuffer buffer, uint offset, Inner value) {
  buffer.Store((offset + 0u), asuint(value.scalar_i32));
  buffer.Store((offset + 4u), asuint(value.scalar_f32));
  buffer.Store<float16_t>((offset + 8u), value.scalar_f16);
}

void tint_symbol_37(RWByteAddressBuffer buffer, uint offset, Inner value[4]) {
  Inner array_3[4] = value;
  {
    for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      tint_symbol_36(buffer, (offset + (i_2 * 12u)), array_3[i_2]);
    }
  }
}

[numthreads(1, 1, 1)]
void main() {
  sb.Store(0u, asuint(0.0f));
  sb.Store(4u, asuint(0));
  sb.Store(8u, asuint(0u));
  sb.Store<float16_t>(12u, float16_t(0.0h));
  sb.Store2(16u, asuint((0.0f).xx));
  sb.Store2(24u, asuint((0).xx));
  sb.Store2(32u, asuint((0u).xx));
  sb.Store<vector<float16_t, 2> >(40u, (float16_t(0.0h)).xx);
  sb.Store3(48u, asuint((0.0f).xxx));
  sb.Store3(64u, asuint((0).xxx));
  sb.Store3(80u, asuint((0u).xxx));
  sb.Store<vector<float16_t, 3> >(96u, (float16_t(0.0h)).xxx);
  sb.Store4(112u, asuint((0.0f).xxxx));
  sb.Store4(128u, asuint((0).xxxx));
  sb.Store4(144u, asuint((0u).xxxx));
  sb.Store<vector<float16_t, 4> >(160u, (float16_t(0.0h)).xxxx);
  tint_symbol_16(sb, 168u, float2x2((0.0f).xx, (0.0f).xx));
  tint_symbol_17(sb, 192u, float2x3((0.0f).xxx, (0.0f).xxx));
  tint_symbol_18(sb, 224u, float2x4((0.0f).xxxx, (0.0f).xxxx));
  tint_symbol_19(sb, 256u, float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  tint_symbol_20(sb, 288u, float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  tint_symbol_21(sb, 336u, float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  tint_symbol_22(sb, 384u, float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  tint_symbol_23(sb, 416u, float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  tint_symbol_24(sb, 480u, float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  tint_symbol_25(sb, 544u, matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  tint_symbol_26(sb, 552u, matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  tint_symbol_27(sb, 568u, matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  tint_symbol_28(sb, 584u, matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  tint_symbol_29(sb, 600u, matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  tint_symbol_30(sb, 624u, matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  tint_symbol_31(sb, 648u, matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  tint_symbol_32(sb, 664u, matrix<float16_t, 4, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  tint_symbol_33(sb, 696u, matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  const float3 tint_symbol_38[2] = (float3[2])0;
  tint_symbol_34(sb, 736u, tint_symbol_38);
  const matrix<float16_t, 4, 2> tint_symbol_39[2] = (matrix<float16_t, 4, 2>[2])0;
  tint_symbol_35(sb, 768u, tint_symbol_39);
  const Inner tint_symbol_40 = (Inner)0;
  tint_symbol_36(sb, 800u, tint_symbol_40);
  const Inner tint_symbol_41[4] = (Inner[4])0;
  tint_symbol_37(sb, 812u, tint_symbol_41);
  return;
}
