
ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);
void v(uint offset, float obj[4]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 4u)) {
        break;
      }
      tint_symbol_1.Store((offset + (v_2 * 4u)), asuint(obj[v_2]));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}

typedef float ary_ret[4];
ary_ret v_3(uint offset) {
  float a[4] = (float[4])0;
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = asfloat(tint_symbol.Load((offset + (v_5 * 4u))));
      {
        v_4 = (v_5 + 1u);
      }
      continue;
    }
  }
  float v_6[4] = a;
  return v_6;
}

[numthreads(1, 1, 1)]
void main() {
  float v_7[4] = v_3(0u);
  v(0u, v_7);
}

