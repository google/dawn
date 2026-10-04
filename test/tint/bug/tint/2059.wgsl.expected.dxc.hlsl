struct S2 {
  float3x3 m[1];
};

struct S {
  float3x3 m;
};

struct S4 {
  S s[1];
};

struct S3 {
  S s;
};


RWByteAddressBuffer buffer0 : register(u0);
RWByteAddressBuffer buffer1 : register(u1);
RWByteAddressBuffer buffer2 : register(u2);
RWByteAddressBuffer buffer3 : register(u3);
RWByteAddressBuffer buffer4 : register(u4);
RWByteAddressBuffer buffer5 : register(u5);
RWByteAddressBuffer buffer6 : register(u6);
RWByteAddressBuffer buffer7 : register(u7);
void v(uint offset, float3x3 obj) {
  buffer7.Store3((offset + 0u), asuint(obj[0u]));
  buffer7.Store3((offset + 16u), asuint(obj[1u]));
  buffer7.Store3((offset + 32u), asuint(obj[2u]));
}

void v_1(uint offset, float3x3 obj[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 1u)) {
        break;
      }
      v((offset + (v_2 * 48u)), obj[v_2]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_3(uint offset, S2 obj) {
  float3x3 v_4[1] = obj.m;
  v_1((offset + 0u), v_4);
}

void v_5(uint offset, S2 obj[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_6 = idx;
      if ((v_6 >= 1u)) {
        break;
      }
      S2 v_7 = obj[v_6];
      v_3((offset + (v_6 * 48u)), v_7);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_8(uint offset, float3x3 obj) {
  buffer6.Store3((offset + 0u), asuint(obj[0u]));
  buffer6.Store3((offset + 16u), asuint(obj[1u]));
  buffer6.Store3((offset + 32u), asuint(obj[2u]));
}

void v_9(uint offset, S obj) {
  v_8((offset + 0u), obj.m);
}

void v_10(uint offset, S obj[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_11 = idx;
      if ((v_11 >= 1u)) {
        break;
      }
      S v_12 = obj[v_11];
      v_9((offset + (v_11 * 48u)), v_12);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_13(uint offset, float3x3 obj) {
  buffer5.Store3((offset + 0u), asuint(obj[0u]));
  buffer5.Store3((offset + 16u), asuint(obj[1u]));
  buffer5.Store3((offset + 32u), asuint(obj[2u]));
}

void v_14(uint offset, float3x3 obj[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_15 = idx;
      if ((v_15 >= 1u)) {
        break;
      }
      v_13((offset + (v_15 * 48u)), obj[v_15]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_16(uint offset, float3x3 obj) {
  buffer4.Store3((offset + 0u), asuint(obj[0u]));
  buffer4.Store3((offset + 16u), asuint(obj[1u]));
  buffer4.Store3((offset + 32u), asuint(obj[2u]));
}

void v_17(uint offset, S obj) {
  v_16((offset + 0u), obj.m);
}

void v_18(uint offset, S obj[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_19 = idx;
      if ((v_19 >= 1u)) {
        break;
      }
      S v_20 = obj[v_19];
      v_17((offset + (v_19 * 48u)), v_20);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_21(uint offset, S4 obj) {
  S v_22[1] = obj.s;
  v_18((offset + 0u), v_22);
}

void v_23(uint offset, float3x3 obj) {
  buffer3.Store3((offset + 0u), asuint(obj[0u]));
  buffer3.Store3((offset + 16u), asuint(obj[1u]));
  buffer3.Store3((offset + 32u), asuint(obj[2u]));
}

void v_24(uint offset, S obj) {
  v_23((offset + 0u), obj.m);
}

void v_25(uint offset, S3 obj) {
  S v_26 = obj.s;
  v_24((offset + 0u), v_26);
}

void v_27(uint offset, float3x3 obj) {
  buffer2.Store3((offset + 0u), asuint(obj[0u]));
  buffer2.Store3((offset + 16u), asuint(obj[1u]));
  buffer2.Store3((offset + 32u), asuint(obj[2u]));
}

void v_28(uint offset, float3x3 obj[1]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_29 = idx;
      if ((v_29 >= 1u)) {
        break;
      }
      v_27((offset + (v_29 * 48u)), obj[v_29]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_30(uint offset, S2 obj) {
  float3x3 v_31[1] = obj.m;
  v_28((offset + 0u), v_31);
}

void v_32(uint offset, float3x3 obj) {
  buffer1.Store3((offset + 0u), asuint(obj[0u]));
  buffer1.Store3((offset + 16u), asuint(obj[1u]));
  buffer1.Store3((offset + 32u), asuint(obj[2u]));
}

void v_33(uint offset, S obj) {
  v_32((offset + 0u), obj.m);
}

void v_34(uint offset, float3x3 obj) {
  buffer0.Store3((offset + 0u), asuint(obj[0u]));
  buffer0.Store3((offset + 16u), asuint(obj[1u]));
  buffer0.Store3((offset + 32u), asuint(obj[2u]));
}

[numthreads(1, 1, 1)]
void main() {
  float3x3 m = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  {
    uint c = 0u;
    while(true) {
      if ((c < 3u)) {
      } else {
        break;
      }
      uint v_35 = c;
      float v_36 = float(((c * 3u) + 1u));
      float v_37 = float(((c * 3u) + 2u));
      m[v_35] = float3(v_36, v_37, float(((c * 3u) + 3u)));
      {
        c = (c + 1u);
      }
    }
  }
  float3x3 a = m;
  v_34(0u, a);
  S a_1 = {m};
  v_33(0u, a_1);
  float3x3 v_38[1] = {m};
  S2 a_2 = {v_38};
  v_30(0u, a_2);
  S v_39 = {m};
  S3 a_3 = {v_39};
  v_25(0u, a_3);
  S v_40 = {m};
  S v_41[1] = {v_40};
  S4 a_4 = {v_41};
  v_21(0u, a_4);
  float3x3 a_5[1] = {m};
  v_14(0u, a_5);
  S v_42 = {m};
  S a_6[1] = {v_42};
  v_10(0u, a_6);
  float3x3 v_43[1] = {m};
  S2 v_44 = {v_43};
  S2 a_7[1] = {v_44};
  v_5(0u, a_7);
}

