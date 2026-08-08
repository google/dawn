struct S {
  float3x3 a[3];
  float3 b[3][3];
};


RWByteAddressBuffer s : register(u0);
void v(uint offset, float3 obj[3]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 3u)) {
        break;
      }
      s.Store3((offset + (v_2 * 16u)), asuint(obj[v_2]));
      {
        v_1 = (v_2 + 1u);
      }
    }
  }
}

void v_3(uint offset, float3 obj[3][3]) {
  {
    uint v_4 = 0u;
    v_4 = 0u;
    while(true) {
      uint v_5 = v_4;
      if ((v_5 >= 3u)) {
        break;
      }
      float3 v_6[3] = obj[v_5];
      v((offset + (v_5 * 48u)), v_6);
      {
        v_4 = (v_5 + 1u);
      }
    }
  }
}

void v_7(uint offset, float3x3 obj) {
  s.Store3((offset + 0u), asuint(obj[0u]));
  s.Store3((offset + 16u), asuint(obj[1u]));
  s.Store3((offset + 32u), asuint(obj[2u]));
}

void v_8(uint offset, float3x3 obj[3]) {
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 3u)) {
        break;
      }
      v_7((offset + (v_10 * 48u)), obj[v_10]);
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
}

void v_11(uint offset, S obj) {
  float3x3 v_12[3] = obj.a;
  v_8((offset + 0u), v_12);
  float3 v_13[3][3] = obj.b;
  v_3((offset + 144u), v_13);
}

typedef float3 ary_ret[3];
ary_ret v_14(uint offset) {
  float3 a[3] = (float3[3])0;
  {
    uint v_15 = 0u;
    v_15 = 0u;
    while(true) {
      uint v_16 = v_15;
      if ((v_16 >= 3u)) {
        break;
      }
      a[v_16] = asfloat(s.Load3((offset + (v_16 * 16u))));
      {
        v_15 = (v_16 + 1u);
      }
    }
  }
  float3 v_17[3] = a;
  return v_17;
}

typedef float3 ary_ret_1[3][3];
ary_ret_1 v_18(uint offset) {
  float3 a[3][3] = (float3[3][3])0;
  {
    uint v_19 = 0u;
    v_19 = 0u;
    while(true) {
      uint v_20 = v_19;
      if ((v_20 >= 3u)) {
        break;
      }
      float3 v_21[3] = v_14((offset + (v_20 * 48u)));
      a[v_20] = v_21;
      {
        v_19 = (v_20 + 1u);
      }
    }
  }
  float3 v_22[3][3] = a;
  return v_22;
}

float3x3 v_23(uint offset) {
  return float3x3(asfloat(s.Load3((offset + 0u))), asfloat(s.Load3((offset + 16u))), asfloat(s.Load3((offset + 32u))));
}

typedef float3x3 ary_ret_2[3];
ary_ret_2 v_24(uint offset) {
  float3x3 a[3] = (float3x3[3])0;
  {
    uint v_25 = 0u;
    v_25 = 0u;
    while(true) {
      uint v_26 = v_25;
      if ((v_26 >= 3u)) {
        break;
      }
      a[v_26] = v_23((offset + (v_26 * 48u)));
      {
        v_25 = (v_26 + 1u);
      }
    }
  }
  float3x3 v_27[3] = a;
  return v_27;
}

S v_28(uint offset) {
  float3x3 v_29[3] = v_24((offset + 0u));
  float3 v_30[3][3] = v_18((offset + 144u));
  S v_31 = {v_29, v_30};
  return v_31;
}

[numthreads(1, 1, 1)]
void main() {
  S v_32 = v_28(0u);
  v_11(0u, v_32);
}

