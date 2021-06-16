[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct tint_array_wrapper {
  int arr[4];
};
struct S {
  tint_array_wrapper arr;
};

tint_array_wrapper tint_symbol_2(RWByteAddressBuffer buffer, uint offset) {
  const tint_array_wrapper tint_symbol_3 = {{asint(buffer.Load((offset + 0u))), asint(buffer.Load((offset + 16u))), asint(buffer.Load((offset + 32u))), asint(buffer.Load((offset + 48u)))}};
  return tint_symbol_3;
}

static tint_array_wrapper src_private;
groupshared tint_array_wrapper src_workgroup;
ConstantBuffer<S> src_uniform : register(b0, space0);
RWByteAddressBuffer src_storage : register(u1, space0);
static tint_array_wrapper tint_symbol;

struct tint_array_wrapper_3 {
  int arr[2];
};
struct tint_array_wrapper_2 {
  tint_array_wrapper_3 arr[3];
};
struct tint_array_wrapper_1 {
  tint_array_wrapper_2 arr[4];
};

static tint_array_wrapper_1 dst_nested;

tint_array_wrapper ret_arr() {
  const tint_array_wrapper tint_symbol_4 = {{0, 0, 0, 0}};
  return tint_symbol_4;
}

S ret_struct_arr() {
  const S tint_symbol_5 = {{{0, 0, 0, 0}}};
  return tint_symbol_5;
}

void foo(tint_array_wrapper src_param) {
  tint_array_wrapper src_function = {{0, 0, 0, 0}};
  const tint_array_wrapper tint_symbol_6 = {{1, 2, 3, 3}};
  tint_symbol = tint_symbol_6;
  tint_symbol = src_param;
  tint_symbol = ret_arr();
  const tint_array_wrapper src_let = {{0, 0, 0, 0}};
  tint_symbol = src_let;
  tint_symbol = src_function;
  tint_symbol = src_private;
  tint_symbol = src_workgroup;
  tint_symbol = ret_struct_arr().arr;
  tint_symbol = src_uniform.arr;
  tint_symbol = tint_symbol_2(src_storage, 0u);
  tint_array_wrapper_1 src_nested = {{{{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}, {{{{0, 0}}, {{0, 0}}, {{0, 0}}}}}};
  dst_nested = src_nested;
}
