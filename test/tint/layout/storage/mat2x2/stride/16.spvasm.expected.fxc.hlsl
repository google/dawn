struct strided_arr {
  float2 el;
};

RWByteAddressBuffer ssbo : register(u0, space0);

float2x2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

typedef strided_arr mat2x2_stride_16_to_arr_ret[2];
mat2x2_stride_16_to_arr_ret mat2x2_stride_16_to_arr(float2x2 m) {
  const strided_arr tint_symbol_7 = {m[0u]};
  const strided_arr tint_symbol_8 = {m[1u]};
  const strided_arr tint_symbol_9[2] = {tint_symbol_7, tint_symbol_8};
  return tint_symbol_9;
}

strided_arr tint_symbol_2(RWByteAddressBuffer buffer, uint offset) {
  const strided_arr tint_symbol_10 = {asfloat(buffer.Load2((offset + 0u)))};
  return tint_symbol_10;
}

typedef strided_arr tint_symbol_1_ret[2];
tint_symbol_1_ret tint_symbol_1(RWByteAddressBuffer buffer, uint offset) {
  strided_arr arr_1[2] = (strided_arr[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i] = tint_symbol_2(buffer, (offset + (i * 16u)));
    }
  }
  return arr_1;
}

void tint_symbol_5(RWByteAddressBuffer buffer, uint offset, strided_arr value) {
  buffer.Store2((offset + 0u), asuint(value.el));
}

void tint_symbol_4(RWByteAddressBuffer buffer, uint offset, strided_arr value[2]) {
  strided_arr array[2] = value;
  {
    for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      tint_symbol_5(buffer, (offset + (i_1 * 16u)), array[i_1]);
    }
  }
}

void f_1() {
  const float2x2 x_15 = arr_to_mat2x2_stride_16(tint_symbol_1(ssbo, 0u));
  const strided_arr tint_symbol[2] = mat2x2_stride_16_to_arr(x_15);
  tint_symbol_4(ssbo, 0u, tint_symbol);
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
