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
  const tint_array_wrapper tint_symbol_3 = {{make_tint_padded_array_element(asint(buffer.Load((offset + 0u)))), make_tint_padded_array_element(asint(buffer.Load((offset + 16u)))), make_tint_padded_array_element(asint(buffer.Load((offset + 32u)))), make_tint_padded_array_element(asint(buffer.Load((offset + 48u))))}};
  return tint_symbol_3;
}

static tint_array_wrapper src_private;
groupshared tint_array_wrapper src_workgroup;
ConstantBuffer<S> src_uniform : register(b0, space0);
RWByteAddressBuffer src_storage : register(u1, space0);

tint_array_wrapper ret_arr() {
  const tint_array_wrapper tint_symbol_4 = {{make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0)}};
  return tint_symbol_4;
}

S ret_struct_arr() {
  const S tint_symbol_5 = {{{make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0)}}};
  return tint_symbol_5;
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

void foo(tint_array_wrapper src_param) {
  tint_array_wrapper src_function = {{make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0)}};
  tint_array_wrapper tint_symbol = {{make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0)}};
  const tint_array_wrapper tint_symbol_6 = {{make_tint_padded_array_element(1), make_tint_padded_array_element(2), make_tint_padded_array_element(3), make_tint_padded_array_element(3)}};
  tint_symbol = tint_symbol_6;
  tint_symbol = src_param;
  tint_symbol = ret_arr();
  const tint_array_wrapper src_let = {{make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0), make_tint_padded_array_element(0)}};
  tint_symbol = src_let;
  tint_symbol = src_function;
  tint_symbol = src_private;
  tint_symbol = src_workgroup;
  tint_symbol = ret_struct_arr().arr;
  tint_symbol = src_uniform.arr;
  tint_symbol = tint_symbol_2(src_storage, 0u);
  tint_array_wrapper_1 dst_nested = {{{{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}}};
  tint_array_wrapper_1 src_nested = {{{{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}}};
  dst_nested = src_nested;
}
