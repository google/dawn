struct S {
  int before;
  float4x3 m;
  int after;
};

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[48];
};
groupshared S w[4];
float4x3 v(uint start_byte_offset) {
  return float4x3(asfloat(u[(start_byte_offset / 16u)].xyz), asfloat(u[((16u + start_byte_offset) / 16u)].xyz), asfloat(u[((32u + start_byte_offset) / 16u)].xyz), asfloat(u[((48u + start_byte_offset) / 16u)].xyz));
}

S v_1(uint start_byte_offset) {
  int v_2 = asint(u[(start_byte_offset / 16u)][((start_byte_offset & 15u) >> 2u)]);
  float4x3 v_3 = v((16u + start_byte_offset));
  uint v_4 = (128u + start_byte_offset);
  S v_5 = {v_2, v_3, asint(u[(v_4 / 16u)][((v_4 & 15u) >> 2u)])};
  return v_5;
}

typedef S ary_ret[4];
ary_ret v_6(uint start_byte_offset) {
  S a[4] = (S[4])0;
  {
    uint v_7 = 0u;
    v_7 = 0u;
    while(true) {
      uint v_8 = v_7;
      if ((v_8 >= 4u)) {
        break;
      }
      S v_9 = v_1((start_byte_offset + (v_8 * 192u)));
      a[v_8] = v_9;
      {
        v_7 = (v_8 + 1u);
      }
    }
  }
  S v_10[4] = a;
  return v_10;
}

void f_inner(uint tint_local_index) {
  {
    uint v_11 = 0u;
    v_11 = tint_local_index;
    while(true) {
      uint v_12 = v_11;
      if ((v_12 >= 4u)) {
        break;
      }
      S v_13 = (S)0;
      w[v_12] = v_13;
      {
        v_11 = (v_12 + 1u);
      }
    }
  }
  GroupMemoryBarrierWithGroupSync();
  S v_14[4] = v_6(0u);
  w = v_14;
  S v_15 = v_1(384u);
  w[1u] = v_15;
  w[3u].m = v(400u);
  w[1u].m[0u] = asfloat(u[2u].xyz).zxy;
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

