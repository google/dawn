struct S {
  int4 arr[4];
};

struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


static int4 src_private[4] = (int4[4])0;
groupshared int4 src_workgroup[4];
cbuffer cbuffer_src_uniform : register(b0) {
  uint4 src_uniform[4];
};
RWByteAddressBuffer src_storage : register(u1);
typedef int4 ary_ret[4];
ary_ret ret_arr() {
  int4 v[4] = (int4[4])0;
  return v;
}

S ret_struct_arr() {
  S v_1 = (S)0;
  return v_1;
}

typedef int4 ary_ret_1[4];
ary_ret_1 v_2(uint offset) {
  int4 a[4] = (int4[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 4u)) {
        break;
      }
      a[v_3] = asint(src_storage.Load4((offset + (v_3 * 16u))));
      {
        idx = (idx + 1u);
      }
    }
  }
  int4 v_4[4] = a;
  return v_4;
}

typedef int4 ary_ret_2[4];
ary_ret_2 v_5(uint start_byte_offset) {
  int4 a[4] = (int4[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_6 = idx;
      if ((v_6 >= 4u)) {
        break;
      }
      a[v_6] = asint(src_uniform[((start_byte_offset + (v_6 * 16u)) / 16u)]);
      {
        idx = (idx + 1u);
      }
    }
  }
  int4 v_7[4] = a;
  return v_7;
}

void foo(int4 src_param[4]) {
  int4 src_function[4] = (int4[4])0;
  int4 v_8[4] = (int4[4])0;
  int4 v_9[4] = {(int(1)).xxxx, (int(2)).xxxx, (int(3)).xxxx, (int(3)).xxxx};
  v_8 = v_9;
  v_8 = src_param;
  int4 v_10[4] = ret_arr();
  v_8 = v_10;
  int4 src_let[4] = (int4[4])0;
  v_8 = src_let;
  int4 v_11[4] = src_function;
  v_8 = v_11;
  int4 v_12[4] = src_private;
  v_8 = v_12;
  int4 v_13[4] = src_workgroup;
  v_8 = v_13;
  S v_14 = ret_struct_arr();
  int4 v_15[4] = v_14.arr;
  v_8 = v_15;
  int4 v_16[4] = v_5(0u);
  v_8 = v_16;
  int4 v_17[4] = v_2(0u);
  v_8 = v_17;
  int dst_nested[4][3][2] = (int[4][3][2])0;
  int src_nested[4][3][2] = (int[4][3][2])0;
  int v_18[4][3][2] = src_nested;
  dst_nested = v_18;
}

void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_19 = idx;
      if ((v_19 >= 4u)) {
        break;
      }
      src_workgroup[v_19] = (int(0)).xxxx;
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  int4 val[4] = (int4[4])0;
  foo(val);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

