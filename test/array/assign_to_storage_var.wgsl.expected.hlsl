[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct tint_padded_array_element {
  /* 0x0000 */ int el;
  /* 0x0004 */ int tint_pad_0[3];
};

tint_padded_array_element make_tint_padded_array_element(int param_0) {
  tint_padded_array_element output;
  output.el = param_0;
  return output;
}
struct tint_array_wrapper {
  /* 0x0000 */ tint_padded_array_element arr[4];
};
struct S {
  /* 0x0000 */ tint_array_wrapper arr;
};

tint_array_wrapper tint_symbol_2(RWByteAddressBuffer buffer, uint offset) {
  const tint_array_wrapper tint_symbol_9 = {{make_tint_padded_array_element(asint(buffer.Load((offset + 0u)))), make_tint_padded_array_element(asint(buffer.Load((offset + 16u)))), make_tint_padded_array_element(asint(buffer.Load((offset + 32u)))), make_tint_padded_array_element(asint(buffer.Load((offset + 48u))))}};
  return tint_symbol_9;
}

void tint_symbol_4(RWByteAddressBuffer buffer, uint offset, tint_array_wrapper value) {
  buffer.Store((offset + 0u), asuint(value.arr[0u].el));
  buffer.Store((offset + 16u), asuint(value.arr[1u].el));
  buffer.Store((offset + 32u), asuint(value.arr[2u].el));
  buffer.Store((offset + 48u), asuint(value.arr[3u].el));
}

struct tint_array_wrapper_3 {
  /* 0x0000 */ int arr[2];
};
struct tint_array_wrapper_2 {
  /* 0x0000 */ tint_array_wrapper_3 arr[3];
};
struct tint_array_wrapper_1 {
  /* 0x0000 */ tint_array_wrapper_2 arr[4];
};

void tint_symbol_6(RWByteAddressBuffer buffer, uint offset, tint_array_wrapper_3 value) {
  buffer.Store((offset + 0u), asuint(value.arr[0u]));
  buffer.Store((offset + 4u), asuint(value.arr[1u]));
}

void tint_symbol_7(RWByteAddressBuffer buffer, uint offset, tint_array_wrapper_2 value) {
  tint_symbol_6(buffer, (offset + 0u), value.arr[0u]);
  tint_symbol_6(buffer, (offset + 8u), value.arr[1u]);
  tint_symbol_6(buffer, (offset + 16u), value.arr[2u]);
}

void tint_symbol_8(RWByteAddressBuffer buffer, uint offset, tint_array_wrapper_1 value) {
  tint_symbol_7(buffer, (offset + 0u), value.arr[0u]);
  tint_symbol_7(buffer, (offset + 24u), value.arr[1u]);
  tint_symbol_7(buffer, (offset + 48u), value.arr[2u]);
  tint_symbol_7(buffer, (offset + 72u), value.arr[3u]);
}

static tint_array_wrapper src_private;
groupshared tint_array_wrapper src_workgroup;
ConstantBuffer<S> src_uniform : register(b0, space0);
RWByteAddressBuffer src_storage : register(u1, space0);
RWByteAddressBuffer tint_symbol : register(u2, space0);
RWByteAddressBuffer dst_nested : register(u3, space0);

tint_array_wrapper ret_arr() {
  const tint_array_wrapper tint_symbol_10 = {{make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0)}};
  return tint_symbol_10;
}

S ret_struct_arr() {
  const S tint_symbol_11 = {{{make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0)}}};
  return tint_symbol_11;
}

void foo(tint_array_wrapper src_param) {
  tint_array_wrapper src_function = {{make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0)}};
  const tint_array_wrapper tint_symbol_12 = {{make_tint_padded_array_element(1), make_tint_padded_array_element(2), make_tint_padded_array_element(3), make_tint_padded_array_element(3)}};
  tint_symbol_4(tint_symbol, 0u, tint_symbol_12);
  tint_symbol_4(tint_symbol, 0u, src_param);
  tint_symbol_4(tint_symbol, 0u, ret_arr());
  const tint_array_wrapper src_let = {{make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0)}};
  tint_symbol_4(tint_symbol, 0u, src_let);
  tint_symbol_4(tint_symbol, 0u, src_function);
  tint_symbol_4(tint_symbol, 0u, src_private);
  tint_symbol_4(tint_symbol, 0u, src_workgroup);
  tint_symbol_4(tint_symbol, 0u, ret_struct_arr().arr);
  tint_symbol_4(tint_symbol, 0u, src_uniform.arr);
  tint_symbol_4(tint_symbol, 0u, tint_symbol_2(src_storage, 0u));
  tint_array_wrapper_1 src_nested = {{{{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}}};
  tint_symbol_8(dst_nested, 0u, src_nested);
}
