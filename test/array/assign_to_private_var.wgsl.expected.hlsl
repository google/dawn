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
static tint_padded_array_element tint_symbol[4] = (tint_padded_array_element[4])0;
static int dst_nested[4][3][2] = (int[4][3][2])0;

typedef tint_padded_array_element ret_arr_ret[4];
ret_arr_ret ret_arr() {
  const tint_padded_array_element tint_symbol_5[4] = (tint_padded_array_element[4])0;
  return tint_symbol_5;
}

S ret_struct_arr() {
  const S tint_symbol_6 = (S)0;
  return tint_symbol_6;
}

typedef tint_padded_array_element tint_symbol_1_ret[4];
tint_symbol_1_ret tint_symbol_1(uint4 buffer[4], uint offset) {
  tint_padded_array_element arr_1[4] = (tint_padded_array_element[4])0;
  {
    [loop] for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      const uint scalar_offset = ((offset + (i * 16u))) / 4;
      arr_1[i].el = asint(buffer[scalar_offset / 4][scalar_offset % 4]);
    }
  }
  return arr_1;
}

typedef tint_padded_array_element tint_symbol_3_ret[4];
tint_symbol_3_ret tint_symbol_3(RWByteAddressBuffer buffer, uint offset) {
  tint_padded_array_element arr_2[4] = (tint_padded_array_element[4])0;
  {
    [loop] for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr_2[i_1].el = asint(buffer.Load((offset + (i_1 * 16u))));
    }
  }
  return arr_2;
}

void foo(tint_padded_array_element src_param[4]) {
  tint_padded_array_element src_function[4] = (tint_padded_array_element[4])0;
  const tint_padded_array_element tint_symbol_7[4] = {{1}, {2}, {3}, {3}};
  tint_symbol = tint_symbol_7;
  tint_symbol = src_param;
  tint_symbol = ret_arr();
  const tint_padded_array_element src_let[4] = (tint_padded_array_element[4])0;
  tint_symbol = src_let;
  tint_symbol = src_function;
  tint_symbol = src_private;
  tint_symbol = src_workgroup;
  tint_symbol = ret_struct_arr().arr;
  tint_symbol = tint_symbol_1(src_uniform, 0u);
  tint_symbol = tint_symbol_3(src_storage, 0u);
  int src_nested[4][3][2] = (int[4][3][2])0;
  dst_nested = src_nested;
}
