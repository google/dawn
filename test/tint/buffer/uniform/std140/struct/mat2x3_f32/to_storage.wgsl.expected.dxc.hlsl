struct S {
  int before;
  float2x3 m;
  int after;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1);
void v(uint offset, float2x3 obj) {
  s.Store3((offset + 0u), asuint(obj[0u]));
  s.Store3((offset + 16u), asuint(obj[1u]));
}

float2x3 v_1(uint start_byte_offset) {
  return float2x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz));
}

void v_2(uint offset, S obj) {
  s.Store((offset + 0u), asuint(obj.before));
  v((offset + 16u), obj.m);
  s.Store((offset + 64u), asuint(obj.after));
}

S v_3(uint start_byte_offset) {
  int v_4 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float2x3 v_5 = v_1((16u + start_byte_offset));
  uint v_6 = (64u + start_byte_offset);
  S v_7 = {v_4, v_5, asint(u[(v_6 / 16u)][((v_6 & 15u) >> 2u)])};
  return v_7;
}

void v_8(uint offset, S obj[4]) {
  {
    uint v_9 = 0u;
    v_9 = 0u;
    while(true) {
      uint v_10 = v_9;
      if ((v_10 >= 4u)) {
        break;
      }
      S v_11 = obj[v_10];
      v_2((offset + (v_10 * 128u)), v_11);
      {
        v_9 = (v_10 + 1u);
      }
    }
  }
}

typedef S ary_ret[4];
ary_ret v_12(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_13 = 0u;
    v_13 = 0u;
    while(true) {
      uint v_14 = v_13;
      if ((v_14 >= 4u)) {
        break;
      }
      S v_15 = v_3((start_byte_offset + (v_14 * 128u)));
      a[v_14] = v_15;
      {
        v_13 = (v_14 + 1u);
      }
    }
  }
  S v_16[4] = a;
  return v_16;
}

[numthreads(1, 1, 1)]
void f() {
  S v_17[4] = v_12(0u);
  v_8(0u, v_17);
  S v_18 = v_3(256u);
  v_2(128u, v_18);
  v(400u, v_1(272u));
  s.Store3(144u, asuint(asfloat(u[2u].xyz).zxy));
}

