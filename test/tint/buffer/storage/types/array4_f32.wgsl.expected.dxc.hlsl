
ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void v_2(uint offset, float obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 4u)) {
        break;
      }
      v_1.Store((offset + (v_3 * 4u)), asuint(obj[v_3]));
      {
        idx = (idx + 1u);
      }
    }
  }
}

typedef float ary_ret[4];
ary_ret v_4(uint offset) {
  float a[4] = (float[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_5 = idx;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = asfloat(v.Load((offset + (v_5 * 4u))));
      {
        idx = (idx + 1u);
      }
    }
  }
  float v_6[4] = a;
  return v_6;
}

[numthreads(1, 1, 1)]
void main() {
  float v_7[4] = v_4(0u);
  v_2(0u, v_7);
}

