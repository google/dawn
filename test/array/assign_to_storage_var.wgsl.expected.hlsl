[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct tint_padded_array_element {
  int el;
};
struct tint_array_wrapper {
  tint_padded_array_element arr[4];
};
struct S {
  tint_array_wrapper arr;
};

tint_array_wrapper tint_symbol_2(uint4 buffer[4], uint offset) {
  const int scalar_offset = ((offset + 0u)) / 4;
  const int scalar_offset_1 = ((offset + 16u)) / 4;
  const int scalar_offset_2 = ((offset + 32u)) / 4;
  const int scalar_offset_3 = ((offset + 48u)) / 4;
  const tint_array_wrapper tint_symbol_11 = {{{asint(buffer[scalar_offset / 4][scalar_offset % 4])}, {asint(buffer[scalar_offset_1 / 4][scalar_offset_1 % 4])}, {asint(buffer[scalar_offset_2 / 4][scalar_offset_2 % 4])}, {asint(buffer[scalar_offset_3 / 4][scalar_offset_3 % 4])}}};
  return tint_symbol_11;
}

tint_array_wrapper tint_symbol_4(RWByteAddressBuffer buffer, uint offset) {
  const tint_array_wrapper tint_symbol_12 = {{{asint(buffer.Load((offset + 0u)))}, {asint(buffer.Load((offset + 16u)))}, {asint(buffer.Load((offset + 32u)))}, {asint(buffer.Load((offset + 48u)))}}};
  return tint_symbol_12;
}

void tint_symbol_6(RWByteAddressBuffer buffer, uint offset, tint_array_wrapper value) {
  buffer.Store((offset + 0u), asuint(value.arr[0u].el));
  buffer.Store((offset + 16u), asuint(value.arr[1u].el));
  buffer.Store((offset + 32u), asuint(value.arr[2u].el));
  buffer.Store((offset + 48u), asuint(value.arr[3u].el));
}

struct tint_array_wrapper_3 {
  int arr[2];
};
struct tint_array_wrapper_2 {
  tint_array_wrapper_3 arr[3];
};
struct tint_array_wrapper_1 {
  tint_array_wrapper_2 arr[4];
};

void tint_symbol_8(RWByteAddressBuffer buffer, uint offset, tint_array_wrapper_3 value) {
  buffer.Store((offset + 0u), asuint(value.arr[0u]));
  buffer.Store((offset + 4u), asuint(value.arr[1u]));
}

void tint_symbol_9(RWByteAddressBuffer buffer, uint offset, tint_array_wrapper_2 value) {
  tint_symbol_8(buffer, (offset + 0u), value.arr[0u]);
  tint_symbol_8(buffer, (offset + 8u), value.arr[1u]);
  tint_symbol_8(buffer, (offset + 16u), value.arr[2u]);
}

void tint_symbol_10(RWByteAddressBuffer buffer, uint offset, tint_array_wrapper_1 value) {
  tint_symbol_9(buffer, (offset + 0u), value.arr[0u]);
  tint_symbol_9(buffer, (offset + 24u), value.arr[1u]);
  tint_symbol_9(buffer, (offset + 48u), value.arr[2u]);
  tint_symbol_9(buffer, (offset + 72u), value.arr[3u]);
}

static tint_array_wrapper src_private = {{{0}, {0}, {0}, {0}}};
groupshared tint_array_wrapper src_workgroup;
cbuffer cbuffer_src_uniform : register(b0, space0) {
  uint4 src_uniform[4];
};
RWByteAddressBuffer src_storage : register(u1, space0);
RWByteAddressBuffer tint_symbol : register(u2, space0);
RWByteAddressBuffer dst_nested : register(u3, space0);

tint_array_wrapper ret_arr() {
  const tint_array_wrapper tint_symbol_13 = {{{0}, {0}, {0}, {0}}};
  return tint_symbol_13;
}

S ret_struct_arr() {
  const S tint_symbol_14 = {{{{0}, {0}, {0}, {0}}}};
  return tint_symbol_14;
}

void foo(tint_array_wrapper src_param) {
  tint_array_wrapper src_function = {{{0}, {0}, {0}, {0}}};
  const tint_array_wrapper tint_symbol_15 = {{{1}, {2}, {3}, {3}}};
  tint_symbol_6(tint_symbol, 0u, tint_symbol_15);
  tint_symbol_6(tint_symbol, 0u, src_param);
  tint_symbol_6(tint_symbol, 0u, ret_arr());
  const tint_array_wrapper src_let = {{{0}, {0}, {0}, {0}}};
  tint_symbol_6(tint_symbol, 0u, src_let);
  tint_symbol_6(tint_symbol, 0u, src_function);
  tint_symbol_6(tint_symbol, 0u, src_private);
  tint_symbol_6(tint_symbol, 0u, src_workgroup);
  tint_symbol_6(tint_symbol, 0u, ret_struct_arr().arr);
  tint_symbol_6(tint_symbol, 0u, tint_symbol_2(src_uniform, 0u));
  tint_symbol_6(tint_symbol, 0u, tint_symbol_4(src_storage, 0u));
  tint_array_wrapper_1 src_nested = {{{{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}}};
  tint_symbol_10(dst_nested, 0u, src_nested);
}
