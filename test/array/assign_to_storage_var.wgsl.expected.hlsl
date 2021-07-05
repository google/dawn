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
  const tint_padded_array_element tint_symbol_11[4] = {{asint(buffer[scalar_offset / 4][scalar_offset % 4])}, {asint(buffer[scalar_offset_1 / 4][scalar_offset_1 % 4])}, {asint(buffer[scalar_offset_2 / 4][scalar_offset_2 % 4])}, {asint(buffer[scalar_offset_3 / 4][scalar_offset_3 % 4])}};
  return tint_symbol_11;
}

typedef tint_padded_array_element tint_symbol_4_ret[4];
tint_symbol_4_ret tint_symbol_4(RWByteAddressBuffer buffer, uint offset) {
  const tint_padded_array_element tint_symbol_12[4] = {{asint(buffer.Load((offset + 0u)))}, {asint(buffer.Load((offset + 16u)))}, {asint(buffer.Load((offset + 32u)))}, {asint(buffer.Load((offset + 48u)))}};
  return tint_symbol_12;
}

void tint_symbol_6(RWByteAddressBuffer buffer, uint offset, tint_padded_array_element value[4]) {
  buffer.Store((offset + 0u), asuint(value[0u].el));
  buffer.Store((offset + 16u), asuint(value[1u].el));
  buffer.Store((offset + 32u), asuint(value[2u].el));
  buffer.Store((offset + 48u), asuint(value[3u].el));
}

void tint_symbol_8(RWByteAddressBuffer buffer, uint offset, int value[2]) {
  buffer.Store((offset + 0u), asuint(value[0u]));
  buffer.Store((offset + 4u), asuint(value[1u]));
}

void tint_symbol_9(RWByteAddressBuffer buffer, uint offset, int value[3][2]) {
  tint_symbol_8(buffer, (offset + 0u), value[0u]);
  tint_symbol_8(buffer, (offset + 8u), value[1u]);
  tint_symbol_8(buffer, (offset + 16u), value[2u]);
}

void tint_symbol_10(RWByteAddressBuffer buffer, uint offset, int value[4][3][2]) {
  tint_symbol_9(buffer, (offset + 0u), value[0u]);
  tint_symbol_9(buffer, (offset + 24u), value[1u]);
  tint_symbol_9(buffer, (offset + 48u), value[2u]);
  tint_symbol_9(buffer, (offset + 72u), value[3u]);
}

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
  const tint_padded_array_element tint_symbol_13[4] = (tint_padded_array_element[4])0;
  return tint_symbol_13;
}

S ret_struct_arr() {
  const S tint_symbol_14 = (S)0;
  return tint_symbol_14;
}

void foo(tint_padded_array_element src_param[4]) {
  tint_padded_array_element src_function[4] = (tint_padded_array_element[4])0;
  const tint_padded_array_element tint_symbol_15[4] = {{1}, {2}, {3}, {3}};
  tint_symbol_6(tint_symbol, 0u, tint_symbol_15);
  tint_symbol_6(tint_symbol, 0u, src_param);
  tint_symbol_6(tint_symbol, 0u, ret_arr());
  const tint_padded_array_element src_let[4] = (tint_padded_array_element[4])0;
  tint_symbol_6(tint_symbol, 0u, src_let);
  tint_symbol_6(tint_symbol, 0u, src_function);
  tint_symbol_6(tint_symbol, 0u, src_private);
  tint_symbol_6(tint_symbol, 0u, src_workgroup);
  tint_symbol_6(tint_symbol, 0u, ret_struct_arr().arr);
  tint_symbol_6(tint_symbol, 0u, tint_symbol_2(src_uniform, 0u));
  tint_symbol_6(tint_symbol, 0u, tint_symbol_4(src_storage, 0u));
  int src_nested[4][3][2] = (int[4][3][2])0;
  tint_symbol_10(dst_nested, 0u, src_nested);
}
