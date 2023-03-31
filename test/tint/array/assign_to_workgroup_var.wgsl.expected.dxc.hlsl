[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

struct S {
  int4 arr[4];
};

static int4 src_private[4] = (int4[4])0;
groupshared int4 src_workgroup[4];
cbuffer cbuffer_src_uniform : register(b0) {
  uint4 src_uniform[4];
};
RWByteAddressBuffer src_storage : register(u1);
groupshared int4 tint_symbol[4];
groupshared int dst_nested[4][3][2];

typedef int4 ret_arr_ret[4];
ret_arr_ret ret_arr() {
  const int4 tint_symbol_2[4] = (int4[4])0;
  return tint_symbol_2;
}

S ret_struct_arr() {
  const S tint_symbol_3 = (S)0;
  return tint_symbol_3;
}

typedef int4 src_uniform_load_ret[4];
src_uniform_load_ret src_uniform_load(uint offset) {
  int4 arr_1[4] = (int4[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      const uint scalar_offset = ((offset + (i * 16u))) / 4;
      arr_1[i] = asint(src_uniform[scalar_offset / 4]);
    }
  }
  return arr_1;
}

typedef int4 src_storage_load_ret[4];
src_storage_load_ret src_storage_load(uint offset) {
  int4 arr_2[4] = (int4[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr_2[i_1] = asint(src_storage.Load4((offset + (i_1 * 16u))));
    }
  }
  return arr_2;
}

void foo(int4 src_param[4]) {
  int4 src_function[4] = (int4[4])0;
  const int4 tint_symbol_4[4] = {(1).xxxx, (2).xxxx, (3).xxxx, (3).xxxx};
  tint_symbol = tint_symbol_4;
  tint_symbol = src_param;
  tint_symbol = ret_arr();
  const int4 src_let[4] = (int4[4])0;
  tint_symbol = src_let;
  tint_symbol = src_function;
  tint_symbol = src_private;
  tint_symbol = src_workgroup;
  const S tint_symbol_1 = ret_struct_arr();
  tint_symbol = tint_symbol_1.arr;
  tint_symbol = src_uniform_load(0u);
  tint_symbol = src_storage_load(0u);
  int src_nested[4][3][2] = (int[4][3][2])0;
  dst_nested = src_nested;
}
