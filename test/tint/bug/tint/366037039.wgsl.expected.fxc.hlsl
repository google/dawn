struct S {
  uint3 a;
  uint b;
  uint3 c[4];
};

struct foo_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_ubuffer : register(b0) {
  uint4 ubuffer[5];
};
RWByteAddressBuffer sbuffer : register(u1);
groupshared S wbuffer;
void v(uint offset, uint3 obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_1 = idx;
      if ((v_1 >= 4u)) {
        break;
      }
      sbuffer.Store3((offset + (v_1 * 16u)), obj[v_1]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_2(uint offset, S obj) {
  sbuffer.Store3((offset + 0u), obj.a);
  sbuffer.Store((offset + 12u), obj.b);
  uint3 v_3[4] = obj.c;
  v((offset + 16u), v_3);
}

typedef uint3 ary_ret[4];
ary_ret v_4(uint offset) {
  uint3 a[4] = (uint3[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_5 = idx;
      if ((v_5 >= 4u)) {
        break;
      }
      a[v_5] = sbuffer.Load3((offset + (v_5 * 16u)));
      {
        idx = (idx + 1u);
      }
    }
  }
  uint3 v_6[4] = a;
  return v_6;
}

S v_7(uint offset) {
  uint3 v_8 = sbuffer.Load3((offset + 0u));
  uint v_9 = sbuffer.Load((offset + 12u));
  uint3 v_10[4] = v_4((offset + 16u));
  S v_11 = {v_8, v_9, v_10};
  return v_11;
}

typedef uint3 ary_ret_1[4];
ary_ret_1 v_12(uint start_byte_offset) {
  uint3 a[4] = (uint3[4])0;
  {
    uint idx = 0u;
    while(true) {
      uint v_13 = idx;
      if ((v_13 >= 4u)) {
        break;
      }
      a[v_13] = ubuffer[((start_byte_offset + (v_13 * 16u)) / 16u)].xyz;
      {
        idx = (idx + 1u);
      }
    }
  }
  uint3 v_14[4] = a;
  return v_14;
}

S v_15(uint start_byte_offset) {
  uint3 v_16 = ubuffer[(start_byte_offset / 16u)].xyz;
  uint v_17 = (12u + start_byte_offset);
  uint v_18 = ubuffer[(v_17 / 16u)][((v_17 & 15u) >> 2u)];
  uint3 v_19[4] = v_12((16u + start_byte_offset));
  S v_20 = {v_16, v_18, v_19};
  return v_20;
}

void foo_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    wbuffer.a = (0u).xxx;
    wbuffer.b = 0u;
  }
  {
    uint idx = tint_local_index;
    while(true) {
      uint v_21 = idx;
      if ((v_21 >= 4u)) {
        break;
      }
      wbuffer.c[v_21] = (0u).xxx;
      {
        idx = (idx + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S u = v_15(0u);
  S s = v_7(0u);
  S w = v_7(0u);
  S v_22 = (S)0;
  v_2(0u, v_22);
  wbuffer = v_22;
}

[numthreads(1, 1, 1)]
void foo(foo_inputs inputs) {
  foo_inner(inputs.tint_local_index);
}

