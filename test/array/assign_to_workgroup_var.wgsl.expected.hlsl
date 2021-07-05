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

typedef tint_padded_array_element tint_symbol_2_ret[4];
tint_symbol_2_ret tint_symbol_2(uint4 buffer[4], uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  const tint_padded_array_element tint_symbol_5[4] = {{asint(buffer[scalar_offset / 4][scalar_offset % 4])}, {asint(buffer[scalar_offset_1 / 4][scalar_offset_1 % 4])}, {asint(buffer[scalar_offset_2 / 4][scalar_offset_2 % 4])}, {asint(buffer[scalar_offset_3 / 4][scalar_offset_3 % 4])}};
  return tint_symbol_5;
}

typedef tint_padded_array_element tint_symbol_4_ret[4];
tint_symbol_4_ret tint_symbol_4(RWByteAddressBuffer buffer, uint offset) {
  const tint_padded_array_element tint_symbol_6[4] = {{asint(buffer.Load((offset + 0u)))}, {asint(buffer.Load((offset + 16u)))}, {asint(buffer.Load((offset + 32u)))}, {asint(buffer.Load((offset + 48u)))}};
  return tint_symbol_6;
}

static tint_padded_array_element src_private[4] = (tint_padded_array_element[4])0;
groupshared tint_padded_array_element src_workgroup[4];
cbuffer cbuffer_src_uniform : register(b0, space0) {
  uint4 src_uniform[4];
};
RWByteAddressBuffer src_storage : register(u1, space0);
groupshared tint_padded_array_element tint_symbol[4];
groupshared int dst_nested[4][3][2];

typedef tint_padded_array_element ret_arr_ret[4];
ret_arr_ret ret_arr() {
  const tint_padded_array_element tint_symbol_7[4] = (tint_padded_array_element[4])0;
  return tint_symbol_7;
}

S ret_struct_arr() {
  const S tint_symbol_8 = (S)0;
  return tint_symbol_8;
}

void foo(tint_padded_array_element src_param[4]) {
  tint_padded_array_element src_function[4] = (tint_padded_array_element[4])0;
  const tint_padded_array_element tint_symbol_9[4] = {{1}, {2}, {3}, {3}};
  tint_symbol = tint_symbol_9;
  tint_symbol = src_param;
  tint_symbol = ret_arr();
  const tint_padded_array_element src_let[4] = (tint_padded_array_element[4])0;
  tint_symbol = src_let;
  tint_symbol = src_function;
  tint_symbol = src_private;
  tint_symbol = src_workgroup;
  tint_symbol = ret_struct_arr().arr;
  tint_symbol = tint_symbol_2(src_uniform, 0u);
  tint_symbol = tint_symbol_4(src_storage, 0u);
  int src_nested[4][3][2] = (int[4][3][2])0;
  dst_nested = src_nested;
}
