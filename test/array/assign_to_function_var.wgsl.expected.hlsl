SKIP: FAILED



Validation Failure:
[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  int arr[4];
};

int[4] tint_symbol_2(RWByteAddressBuffer buffer, uint offset) {
  const int tint_symbol_3[4] = {asint(buffer.Load((offset + 0u))), asint(buffer.Load((offset + 16u))), asint(buffer.Load((offset + 32u))), asint(buffer.Load((offset + 48u)))};
  return tint_symbol_3;
}

static int src_private[4];
groupshared int src_workgroup[4];
ConstantBuffer<S> src_uniform : register(b0, space0);
RWByteAddressBuffer src_storage : register(u1, space0);

int[4] ret_arr() {
  const int tint_symbol_4[4] = {0, 0, 0, 0};
  return tint_symbol_4;
}
S ret_struct_arr() {
  const S tint_symbol_5 = {{0, 0, 0, 0}};
  return tint_symbol_5;
}
void foo(int src_param[4]) {
  int src_function[4] = {0, 0, 0, 0};
  int tint_symbol[4] = {0, 0, 0, 0};
  const int tint_symbol_6[4] = {1, 2, 3, 3};
  tint_symbol = tint_symbol_6;
  tint_symbol = src_param;
  tint_symbol = ret_arr();
  const int src_let[4] = {0, 0, 0, 0};
  tint_symbol = src_let;
  tint_symbol = src_function;
  tint_symbol = src_private;
  tint_symbol = src_workgroup;
  tint_symbol = ret_struct_arr().arr;
  tint_symbol = src_uniform.arr;
  tint_symbol = tint_symbol_2(src_storage, 0u);
  int dst_nested[4][3][2] = {{{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}};
  int src_nested[4][3][2] = {{{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}};
  dst_nested = src_nested;
}

tint_bTL3Zd:10:62: error: brackets are not allowed here; to declare an array, place the brackets after the name
int[4] tint_symbol_2(RWByteAddressBuffer buffer, uint offset) {
   ~~~                                                       ^
                                                             [4]
tint_bTL3Zd:20:17: error: brackets are not allowed here; to declare an array, place the brackets after the name
int[4] ret_arr() {
   ~~~          ^
                [4]

