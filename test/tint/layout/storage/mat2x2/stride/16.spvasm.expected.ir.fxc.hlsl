struct strided_arr {
  float2 el;
};


RWByteAddressBuffer ssbo : register(u0);
typedef strided_arr ary_ret[2];
ary_ret mat2x2_stride_16_to_arr(float2x2 m) {
  strided_arr v = {m[0u]};
  strided_arr v_1 = v;
  strided_arr v_2 = {m[1u]};
  strided_arr v_3[2] = {v_1, v_2};
  return v_3;
}

float2x2 arr_to_mat2x2_stride_16(strided_arr arr[2]) {
  return float2x2(arr[0u].el, arr[1u].el);
}

void v_4(uint offset, strided_arr obj) {
  ssbo.Store2((offset + 0u), asuint(obj.el));
}

void v_5(uint offset, strided_arr obj[2]) {
  {
    uint v_6 = 0u;
    v_6 = 0u;
    while(true) {
      uint v_7 = v_6;
      if ((v_7 >= 2u)) {
        break;
      }
      strided_arr v_8 = obj[v_7];
      v_4((offset + (v_7 * 16u)), v_8);
      {
        v_6 = (v_7 + 1u);
      }
      continue;
    }
  }
}

strided_arr v_9(uint offset) {
  strided_arr v_10 = {asfloat(ssbo.Load2((offset + 0u)))};
  return v_10;
}

typedef strided_arr ary_ret_1[2];
ary_ret_1 v_11(uint offset) {
  strided_arr a[2] = (strided_arr[2])0;
  {
    uint v_12 = 0u;
    v_12 = 0u;
    while(true) {
      uint v_13 = v_12;
      if ((v_13 >= 2u)) {
        break;
      }
      strided_arr v_14 = v_9((offset + (v_13 * 16u)));
      a[v_13] = v_14;
      {
        v_12 = (v_13 + 1u);
      }
      continue;
    }
  }
  strided_arr v_15[2] = a;
  return v_15;
}

void f_1() {
  strided_arr v_16[2] = v_11(0u);
  strided_arr v_17[2] = mat2x2_stride_16_to_arr(arr_to_mat2x2_stride_16(v_16));
  v_5(0u, v_17);
}

[numthreads(1, 1, 1)]
void f() {
  f_1();
}

