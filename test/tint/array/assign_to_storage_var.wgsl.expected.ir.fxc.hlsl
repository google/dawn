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
RWByteAddressBuffer tint_symbol : register(u2);
RWByteAddressBuffer dst_nested : register(u3);
typedef int4 ary_ret[4];
ary_ret ret_arr() {
  int4 v[4] = (int4[4])0;
  return v;
}

S ret_struct_arr() {
  S v_1 = (S)0;
  return v_1;
}

void v_2(uint offset, int obj[2]) {
  {
    uint v_3 = 0u;
    v_3 = 0u;
    while(true) {
      uint v_4 = v_3;
      if ((v_4 >= 2u)) {
        break;
      }
      dst_nested.Store((offset + (v_4 * 4u)), asuint(obj[v_4]));
      {
        v_3 = (v_4 + 1u);
      }
      continue;
    }
  }
}

void v_5(uint offset, int obj[3][2]) {
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 3u)) {
        break;
      }
      int v_8[2] = obj[v_7];
      v_2((offset + (v_7 * 8u)), v_8);
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
}

void v_9(uint offset, int obj[4][3][2]) {
  {
    uint v_10 = 0u;
    v_10 = 0u;
    while(true) {
      uint v_11 = v_10;
      if ((v_11 >= 4u)) {
        break;
      }
      int v_12[3][2] = obj[v_11];
      v_5((offset + (v_11 * 24u)), v_12);
      {
        v_10 = (v_11 + 1u);
      }
      continue;
    }
  }
}

void v_13(uint offset, int4 obj[4]) {
  {
    uint v_14 = 0u;
    v_14 = 0u;
    while(true) {
      uint v_15 = v_14;
      if ((v_15 >= 4u)) {
        break;
      }
      tint_symbol.Store4((offset + (v_15 * 16u)), asuint(obj[v_15]));
      {
        v_14 = (v_15 + 1u);
      }
      continue;
    }
  }
}

typedef int4 ary_ret_1[4];
ary_ret_1 v_16(uint offset) {
  int4 a[4] = (int4[4])0;
  {
    uint v_17 = 0u;
    v_17 = 0u;
    while(true) {
      uint v_18 = v_17;
      if ((v_18 >= 4u)) {
        break;
      }
      a[v_18] = asint(src_storage.Load4((offset + (v_18 * 16u))));
      {
        v_17 = (v_18 + 1u);
      }
      continue;
    }
  }
  int4 v_19[4] = a;
  return v_19;
}

typedef int4 ary_ret_2[4];
ary_ret_2 v_20(uint start_byte_offset) {
  int4 a[4] = (int4[4])0;
  {
    uint v_21 = 0u;
    v_21 = 0u;
    while(true) {
      uint v_22 = v_21;
      if ((v_22 >= 4u)) {
        break;
      }
      a[v_22] = asint(src_uniform[((start_byte_offset + (v_22 * 16u)) / 16u)]);
      {
        v_21 = (v_22 + 1u);
      }
      continue;
    }
  }
  int4 v_23[4] = a;
  return v_23;
}

void foo(int4 src_param[4]) {
  int4 src_function[4] = (int4[4])0;
  int4 v_24[4] = {(int(1)).xxxx, (int(2)).xxxx, (int(3)).xxxx, (int(3)).xxxx};
  v_13(0u, v_24);
  v_13(0u, src_param);
  int4 v_25[4] = ret_arr();
  v_13(0u, v_25);
  int4 src_let[4] = (int4[4])0;
  v_13(0u, src_let);
  int4 v_26[4] = src_function;
  v_13(0u, v_26);
  int4 v_27[4] = src_private;
  v_13(0u, v_27);
  int4 v_28[4] = src_workgroup;
  v_13(0u, v_28);
  S v_29 = ret_struct_arr();
  int4 v_30[4] = v_29.arr;
  v_13(0u, v_30);
  int4 v_31[4] = v_20(0u);
  v_13(0u, v_31);
  int4 v_32[4] = v_16(0u);
  v_13(0u, v_32);
  int src_nested[4][3][2] = (int[4][3][2])0;
  int v_33[4][3][2] = src_nested;
  v_9(0u, v_33);
}

void main_inner(uint tint_local_index) {
  {
    uint v_34 = 0u;
    v_34 = tint_local_index;
    while(true) {
      uint v_35 = v_34;
      if ((v_35 >= 4u)) {
        break;
      }
      src_workgroup[v_35] = (int(0)).xxxx;
      {
        v_34 = (v_35 + 1u);
      }
      continue;
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

