struct strided_arr {
  float2 el;
};

RWByteAddressBuffer ssbo : register(u0, space0);

float2x2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

typedef strided_arr mat2x2_stride_16_to_arr_ret[2];
mat2x2_stride_16_to_arr_ret mat2x2_stride_16_to_arr(float2x2 mat) {
  const strided_arr tint_symbol_6 = {mat[0u]};
  const strided_arr tint_symbol_7 = {mat[1u]};
  const strided_arr tint_symbol_8[2] = {tint_symbol_6, tint_symbol_7};
  return tint_symbol_8;
}

strided_arr tint_symbol_1(RWByteAddressBuffer buffer, uint offset) {
  const strided_arr tint_symbol_9 = {asfloat(buffer.Load2((offset + 0u)))};
  return tint_symbol_9;
}

typedef strided_arr tint_symbol_ret[2];
tint_symbol_ret tint_symbol(RWByteAddressBuffer buffer, uint offset) {
  strided_arr arr_1[2] = (strided_arr[2])0;
  {
    [loop] for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i] = tint_symbol_1(buffer, (offset + (i * 16u)));
    }
  }
  return arr_1;
}

void tint_symbol_4(RWByteAddressBuffer buffer, uint offset, strided_arr value) {
  buffer.Store2((offset + 0u), asuint(value.el));
}

void tint_symbol_3(RWByteAddressBuffer buffer, uint offset, strided_arr value[2]) {
  strided_arr array[2] = value;
  {
    [loop] for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      tint_symbol_4(buffer, (offset + (i_1 * 16u)), array[i_1]);
    }
  }
}

void f_1() {
  const float2x2 x_15 = arr_to_mat2x2_stride_16(tint_symbol(ssbo, 0u));
  tint_symbol_3(ssbo, 0u, mat2x2_stride_16_to_arr(x_15));
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
