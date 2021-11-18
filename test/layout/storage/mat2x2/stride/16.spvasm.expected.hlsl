struct tint_padded_array_element {
  float2 el;
};

RWByteAddressBuffer ssbo : register(u0, space0);

float2x2 arr_to_mat2x2_stride_16(tint_padded_array_element arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

typedef tint_padded_array_element mat2x2_stride_16_to_arr_ret[2];
mat2x2_stride_16_to_arr_ret mat2x2_stride_16_to_arr(float2x2 mat) {
  const tint_padded_array_element tint_symbol_4[2] = {{mat[0u]}, {mat[1u]}};
  return tint_symbol_4;
}

typedef tint_padded_array_element tint_symbol_ret[2];
tint_symbol_ret tint_symbol(RWByteAddressBuffer buffer, uint offset) {
  tint_padded_array_element arr_1[2] = (tint_padded_array_element[2])0;
  {
    [loop] for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      arr_1[i].el = asfloat(buffer.Load2((offset + (i * 16u))));
    }
  }
  return arr_1;
}

void tint_symbol_2(RWByteAddressBuffer buffer, uint offset, tint_padded_array_element value[2]) {
  tint_padded_array_element array[2] = value;
  {
    [loop] for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      buffer.Store2((offset + (i_1 * 16u)), asuint(array[i_1].el));
    }
  }
}

void f_1() {
  const float2x2 x_15 = arr_to_mat2x2_stride_16(tint_symbol(ssbo, 0u));
  tint_symbol_2(ssbo, 0u, mat2x2_stride_16_to_arr(x_15));
  return;
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
  return;
}
