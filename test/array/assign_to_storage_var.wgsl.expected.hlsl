[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct tint_padded_array_element {
  int el;
};
struct S {
  tint_padded_array_element arr[4];
};

static tint_padded_array_element src_private[4] = (tint_padded_array_element[4])0;
groupshared tint_padded_array_element src_workgroup[4];
cbuffer cbuffer_src_uniform : register(b0, space0) {
  uint4 src_uniform[4];
};
RWByteAddressBuffer src_storage : register(u1, space0);
RWByteAddressBuffer tint_symbol : register(u2, space0);
RWByteAddressBuffer dst_nested : register(u3, space0);

typedef tint_padded_array_element ret_arr_ret[4];
ret_arr_ret ret_arr() {
  const tint_padded_array_element tint_symbol_11[4] = (tint_padded_array_element[4])0;
  return tint_symbol_11;
}

S ret_struct_arr() {
  const S tint_symbol_12 = (S)0;
  return tint_symbol_12;
}

void tint_symbol_1(RWByteAddressBuffer buffer, uint offset, tint_padded_array_element value[4]) {
  tint_padded_array_element array[4] = value;
  {
    [loop] for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      buffer.Store((offset + (i * 16u)), asuint(array[i].el));
    }
  }
}

typedef tint_padded_array_element tint_symbol_3_ret[4];
tint_symbol_3_ret tint_symbol_3(uint4 buffer[4], uint offset) {
  tint_padded_array_element arr_1[4] = (tint_padded_array_element[4])0;
  {
    [loop] for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      const uint scalar_offset = ((offset + (i_1 * 16u))) / 4;
      arr_1[i_1].el = asint(buffer[scalar_offset / 4][scalar_offset % 4]);
    }
  }
  return arr_1;
}

typedef tint_padded_array_element tint_symbol_5_ret[4];
tint_symbol_5_ret tint_symbol_5(RWByteAddressBuffer buffer, uint offset) {
  tint_padded_array_element arr_2[4] = (tint_padded_array_element[4])0;
  {
    [loop] for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      arr_2[i_2].el = asint(buffer.Load((offset + (i_2 * 16u))));
    }
  }
  return arr_2;
}

void tint_symbol_9(RWByteAddressBuffer buffer, uint offset, int value[2]) {
  int array_3[2] = value;
  {
    [loop] for(uint i_3 = 0u; (i_3 < 2u); i_3 = (i_3 + 1u)) {
      buffer.Store((offset + (i_3 * 4u)), asuint(array_3[i_3]));
    }
  }
}

void tint_symbol_8(RWByteAddressBuffer buffer, uint offset, int value[3][2]) {
  int array_2[3][2] = value;
  {
    [loop] for(uint i_4 = 0u; (i_4 < 3u); i_4 = (i_4 + 1u)) {
      tint_symbol_9(buffer, (offset + (i_4 * 8u)), array_2[i_4]);
    }
  }
}

void tint_symbol_7(RWByteAddressBuffer buffer, uint offset, int value[4][3][2]) {
  int array_1[4][3][2] = value;
  {
    [loop] for(uint i_5 = 0u; (i_5 < 4u); i_5 = (i_5 + 1u)) {
      tint_symbol_8(buffer, (offset + (i_5 * 24u)), array_1[i_5]);
    }
  }
}

void foo(tint_padded_array_element src_param[4]) {
  tint_padded_array_element src_function[4] = (tint_padded_array_element[4])0;
  const tint_padded_array_element tint_symbol_13[4] = {{1}, {2}, {3}, {3}};
  tint_symbol_1(tint_symbol, 0u, tint_symbol_13);
  tint_symbol_1(tint_symbol, 0u, src_param);
  tint_symbol_1(tint_symbol, 0u, ret_arr());
  const tint_padded_array_element src_let[4] = (tint_padded_array_element[4])0;
  tint_symbol_1(tint_symbol, 0u, src_let);
  tint_symbol_1(tint_symbol, 0u, src_function);
  tint_symbol_1(tint_symbol, 0u, src_private);
  tint_symbol_1(tint_symbol, 0u, src_workgroup);
  tint_symbol_1(tint_symbol, 0u, ret_struct_arr().arr);
  tint_symbol_1(tint_symbol, 0u, tint_symbol_3(src_uniform, 0u));
  tint_symbol_1(tint_symbol, 0u, tint_symbol_5(src_storage, 0u));
  int src_nested[4][3][2] = (int[4][3][2])0;
  tint_symbol_7(dst_nested, 0u, src_nested);
}
