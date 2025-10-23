SKIP: TIMEOUT

struct A {
  float a[1000000];
};


ByteAddressBuffer v : register(t0);
ByteAddressBuffer b : register(t1);
typedef float ary_ret[1000000];
ary_ret v_1(uint offset) {
  float a[1000000] = (float[1000000])0;
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 1000000u)) {
        break;
      }
      a[v_3] = asfloat(b.Load((offset + (v_3 * 4u))));
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
  float v_4[1000000] = a;
  return v_4;
}

A v_5(uint offset) {
  float v_6[1000000] = v_1((offset + 0u));
  A v_7 = {v_6};
  return v_7;
}

typedef int ary_ret_1[1000000];
ary_ret_1 v_8(uint offset) {
  int a[1000000] = (int[1000000])0;
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 1000000u)) {
        break;
      }
      a[v_10] = asint(v.Load((offset + (v_10 * 4u))));
      {
        v_9 = (v_10 + 1u);
      }
      continue;
    }
  }
  int v_11[1000000] = a;
  return v_11;
}

[numthreads(1, 1, 1)]
void main() {
  v_8(0u);
  v_5(0u);
}

