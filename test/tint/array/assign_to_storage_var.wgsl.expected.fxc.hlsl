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
RWByteAddressBuffer v : register(u2);
RWByteAddressBuffer dst_nested : register(u3);
typedef int4 ary_ret[4];
ary_ret ret_arr() {
  int4 v_1[4] = (int4[4])0;
  return v_1;
}

S ret_struct_arr() {
  S v_2 = (S)0;
  return v_2;
}

void v_3(uint offset, int obj[2]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_4 = idx;
      if ((v_4 >= 2u)) {
        break;
      }
      dst_nested.Store((offset + (v_4 * 4u)), asuint(obj[v_4]));
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_5(uint offset, int obj[3][2]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_6 = idx;
      if ((v_6 >= 3u)) {
        break;
      }
      int v_7[2] = obj[v_6];
      v_3((offset + (v_6 * 8u)), v_7);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_8(uint offset, int obj[4][3][2]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_9 = idx;
      if ((v_9 >= 4u)) {
        break;
      }
      int v_10[3][2] = obj[v_9];
      v_5((offset + (v_9 * 24u)), v_10);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_11(uint offset, int4 obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_12 = idx;
      if ((v_12 >= 4u)) {
        break;
      }
      v.Store4((offset + (v_12 * 16u)), asuint(obj[v_12]));
      {
        idx = (idx + 1u);
      }
    }
  }
}

typedef int4 ary_ret_1[4];
ary_ret_1 v_13(uint offset) {
  int4 a[4] = (int4[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_14 = idx;
      if ((v_14 >= 4u)) {
        break;
      }
      a[v_14] = asint(src_storage.Load4((offset + (v_14 * 16u))));
      {
        idx = (idx + 1u);
      }
    }
  }
  int4 v_15[4] = a;
  return v_15;
}

typedef int4 ary_ret_2[4];
ary_ret_2 v_16(uint start_byte_offset) {
  int4 a[4] = (int4[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_17 = idx;
      if ((v_17 >= 4u)) {
        break;
      }
      a[v_17] = asint(src_uniform[((start_byte_offset + (v_17 * 16u)) / 16u)]);
      {
        idx = (idx + 1u);
      }
    }
  }
  int4 v_18[4] = a;
  return v_18;
}

void foo(int4 src_param[4]) {
  int4 src_function[4] = (int4[4])0;
  int4 v_19[4] = {(int(1)).xxxx, (int(2)).xxxx, (int(3)).xxxx, (int(3)).xxxx};
  v_11(0u, v_19);
  v_11(0u, src_param);
  int4 v_20[4] = ret_arr();
  v_11(0u, v_20);
  int4 src_let[4] = (int4[4])0;
  v_11(0u, src_let);
  int4 v_21[4] = src_function;
  v_11(0u, v_21);
  int4 v_22[4] = src_private;
  v_11(0u, v_22);
  int4 v_23[4] = src_workgroup;
  v_11(0u, v_23);
  S v_24 = ret_struct_arr();
  int4 v_25[4] = v_24.arr;
  v_11(0u, v_25);
  int4 v_26[4] = v_16(0u);
  v_11(0u, v_26);
  int4 v_27[4] = v_13(0u);
  v_11(0u, v_27);
  int src_nested[4][3][2] = (int[4][3][2])0;
  int v_28[4][3][2] = src_nested;
  v_8(0u, v_28);
}

void main_inner(uint tint_local_index) {
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_29 = idx;
      if ((v_29 >= 4u)) {
        break;
      }
      src_workgroup[v_29] = (int(0)).xxxx;
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  int4 ary[4] = (int4[4])0;
  foo(ary);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

