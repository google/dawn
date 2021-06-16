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
  const int tint_symbol_9[4] = {asint(buffer.Load((offset + 0u))), asint(buffer.Load((offset + 16u))), asint(buffer.Load((offset + 32u))), asint(buffer.Load((offset + 48u)))};
  return tint_symbol_9;
}
void tint_symbol_4(RWByteAddressBuffer buffer, uint offset, int value[4]) {
  buffer.Store((offset + 0u), asuint(value[0u]));
  buffer.Store((offset + 16u), asuint(value[1u]));
  buffer.Store((offset + 32u), asuint(value[2u]));
  buffer.Store((offset + 48u), asuint(value[3u]));
}

void tint_symbol_6(RWByteAddressBuffer buffer, uint offset, int value[2]) {
  buffer.Store((offset + 0u), asuint(value[0u]));
  buffer.Store((offset + 4u), asuint(value[1u]));
}
void tint_symbol_7(RWByteAddressBuffer buffer, uint offset, int value[3][2]) {
  tint_symbol_6(buffer, (offset + 0u), value[0u]);
  tint_symbol_6(buffer, (offset + 8u), value[1u]);
  tint_symbol_6(buffer, (offset + 16u), value[2u]);
}
void tint_symbol_8(RWByteAddressBuffer buffer, uint offset, int value[4][3][2]) {
  tint_symbol_7(buffer, (offset + 0u), value[0u]);
  tint_symbol_7(buffer, (offset + 24u), value[1u]);
  tint_symbol_7(buffer, (offset + 48u), value[2u]);
  tint_symbol_7(buffer, (offset + 72u), value[3u]);
}

static int src_private[4];
groupshared int src_workgroup[4];
ConstantBuffer<S> src_uniform : register(b0, space0);
RWByteAddressBuffer src_storage : register(u1, space0);
RWByteAddressBuffer tint_symbol : register(u2, space0);
RWByteAddressBuffer dst_nested : register(u3, space0);

int[4] ret_arr() {
  const int tint_symbol_10[4] = {0, 0, 0, 0};
  return tint_symbol_10;
}
S ret_struct_arr() {
  const S tint_symbol_11 = {{0, 0, 0, 0}};
  return tint_symbol_11;
}
void foo(int src_param[4]) {
  int src_function[4] = {0, 0, 0, 0};
  const int tint_symbol_12[4] = {1, 2, 3, 3};
  tint_symbol_4(tint_symbol, 0u, tint_symbol_12);
  tint_symbol_4(tint_symbol, 0u, src_param);
  tint_symbol_4(tint_symbol, 0u, ret_arr());
  const int src_let[4] = {0, 0, 0, 0};
  tint_symbol_4(tint_symbol, 0u, src_let);
  tint_symbol_4(tint_symbol, 0u, src_function);
  tint_symbol_4(tint_symbol, 0u, src_private);
  tint_symbol_4(tint_symbol, 0u, src_workgroup);
  tint_symbol_4(tint_symbol, 0u, ret_struct_arr().arr);
  tint_symbol_4(tint_symbol, 0u, src_uniform.arr);
  tint_symbol_4(tint_symbol, 0u, tint_symbol_2(src_storage, 0u));
  int src_nested[4][3][2] = {{{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}}};
  tint_symbol_8(dst_nested, 0u, src_nested);
}

tint_4gsbi1:10:62: error: brackets are not allowed here; to declare an array, place the brackets after the name
int[4] tint_symbol_2(RWByteAddressBuffer buffer, uint offset) {
   ~~~                                                       ^
                                                             [4]
tint_4gsbi1:44:17: error: brackets are not allowed here; to declare an array, place the brackets after the name
int[4] ret_arr() {
   ~~~          ^
                [4]

