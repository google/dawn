
cbuffer cbuffer_S : register(b0) {
  uint4 S[4];
};
typedef int4 ary_ret[4];
ary_ret v(uint start_byte_offset) {
  int4 a[4] = (int4[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 4u)) {
        break;
      }
      a[v_1] = asint(S[((start_byte_offset + (v_1 * 16u)) / 16u)]);
      {
        idx = (idx + 1u);
      }
    }
  }
  int4 v_2[4] = a;
  return v_2;
}

typedef int4 ary_ret_1[4];
ary_ret_1 func() {
  int4 v_3[4] = v(0u);
  return v_3;
}

[numthreads(1, 1, 1)]
void main() {
  int4 r[4] = func();
}

