
ByteAddressBuffer S : register(t0);
typedef int ary_ret[4];
ary_ret v(uint offset) {
  int a[4] = (int[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 4u)) {
        break;
      }
      a[v_1] = asint(S.Load((offset + (v_1 * 4u))));
      {
        idx = (idx + 1u);
      }
    }
  }
  int v_2[4] = a;
  return v_2;
}

typedef int ary_ret_1[4];
ary_ret_1 func() {
  int v_3[4] = v(0u);
  return v_3;
}

[numthreads(1, 1, 1)]
void main() {
  int r[4] = func();
}

