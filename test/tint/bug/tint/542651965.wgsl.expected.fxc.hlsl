struct S {
  float3x3 a[3];
  float3 b[3][3];
};


RWByteAddressBuffer s : register(u0);
void v(uint offset, float3 obj[3]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 3u)) {
        break;
      }
      s.Store3((offset + (v_1 * 16u)), asuint(obj[v_1]));
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_2(uint offset, float3 obj[3][3]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_3 = idx;
      if ((v_3 >= 3u)) {
        break;
      }
      float3 v_4[3] = obj[v_3];
      v((offset + (v_3 * 48u)), v_4);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_5(uint offset, float3x3 obj) {
  s.Store3((offset + 0u), asuint(obj[0u]));
  s.Store3((offset + 16u), asuint(obj[1u]));
  s.Store3((offset + 32u), asuint(obj[2u]));
}

void v_6(uint offset, float3x3 obj[3]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_7 = idx;
      if ((v_7 >= 3u)) {
        break;
      }
      v_5((offset + (v_7 * 48u)), obj[v_7]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_8(uint offset, S obj) {
  float3x3 v_9[3] = obj.a;
  v_6((offset + 0u), v_9);
  float3 v_10[3][3] = obj.b;
  v_2((offset + 144u), v_10);
}

typedef float3 ary_ret[3];
ary_ret v_11(uint offset) {
  float3 a[3] = (float3[3])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_12 = idx;
      if ((v_12 >= 3u)) {
        break;
      }
      a[v_12] = asfloat(s.Load3((offset + (v_12 * 16u))));
      {
        idx = (idx + 1u);
      }
    }
  }
  float3 v_13[3] = a;
  return v_13;
}

typedef float3 ary_ret_1[3][3];
ary_ret_1 v_14(uint offset) {
  float3 a[3][3] = (float3[3][3])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_15 = idx;
      if ((v_15 >= 3u)) {
        break;
      }
      float3 v_16[3] = v_11((offset + (v_15 * 48u)));
      a[v_15] = v_16;
      {
        idx = (idx + 1u);
      }
    }
  }
  float3 v_17[3][3] = a;
  return v_17;
}

float3x3 v_18(uint offset) {
  return float3x3(asfloat(s.Load3((offset + 0u))), asfloat(s.Load3((offset + 16u))), asfloat(s.Load3((offset + 32u))));
}

typedef float3x3 ary_ret_2[3];
ary_ret_2 v_19(uint offset) {
  float3x3 a[3] = (float3x3[3])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_20 = idx;
      if ((v_20 >= 3u)) {
        break;
      }
      a[v_20] = v_18((offset + (v_20 * 48u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  float3x3 v_21[3] = a;
  return v_21;
}

S v_22(uint offset) {
  float3x3 v_23[3] = v_19((offset + 0u));
  float3 v_24[3][3] = v_14((offset + 144u));
  S v_25 = {v_23, v_24};
  return v_25;
}

[numthreads(1, 1, 1)]
void main() {
  S v_26 = v_22(0u);
  v_8(0u, v_26);
}

