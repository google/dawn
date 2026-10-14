
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void v_2(uint offset, float16_t obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 4u)) {
        break;
      }
      v_1.Store<float16_t>((offset + (v_3 * 2u)), obj[v_3]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

typedef float16_t ary_ret[4];
ary_ret v_4(uint offset) {
  float16_t a[4] = (float16_t[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_5 = idx;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = v.Load<float16_t>((offset + (v_5 * 2u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float16_t v_6[4] = a;
  return v_6;
}

[numthreads(1, 1, 1)]
void main() {
  float16_t v_7[4] = v_4(0u);
  v_2(0u, v_7);
}

