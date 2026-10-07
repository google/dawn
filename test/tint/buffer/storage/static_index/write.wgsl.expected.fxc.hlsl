struct Inner {
  int scalar_i32;
  float scalar_f32;
};


RWByteAddressBuffer sb : register(u0);
void v(uint offset, Inner obj) {
  sb.Store((offset + 0u), asuint(obj.scalar_i32));
  sb.Store((offset + 4u), asuint(obj.scalar_f32));
}

void v_1(uint offset, Inner obj[4]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 4u)) {
        break;
      }
      Inner v_3 = obj[v_2];
      v((offset + (v_2 * 8u)), v_3);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_4(uint offset, float3 obj[2]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_5 = idx;
      if ((v_5 >= 2u)) {
        break;
      }
      sb.Store3((offset + (v_5 * 16u)), asuint(obj[v_5]));
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_6(uint offset, float4x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
  sb.Store4((offset + 48u), asuint(obj[3u]));
}

void v_7(uint offset, float4x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
  sb.Store3((offset + 48u), asuint(obj[3u]));
}

void v_8(uint offset, float4x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
  sb.Store2((offset + 24u), asuint(obj[3u]));
}

void v_9(uint offset, float3x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
}

void v_10(uint offset, float3x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
}

void v_11(uint offset, float3x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
}

void v_12(uint offset, float2x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
}

void v_13(uint offset, float2x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
}

void v_14(uint offset, float2x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
}

[numthreads(1, 1, 1)]
void main() {
  sb.Store(0u, 0u);
  sb.Store(4u, 0u);
  sb.Store(8u, 0u);
  sb.Store2(16u, (0u).xx);
  sb.Store2(24u, (0u).xx);
  sb.Store2(32u, (0u).xx);
  sb.Store3(48u, (0u).xxx);
  sb.Store3(64u, (0u).xxx);
  sb.Store3(80u, (0u).xxx);
  sb.Store4(96u, (0u).xxxx);
  sb.Store4(112u, (0u).xxxx);
  sb.Store4(128u, (0u).xxxx);
  v_14(144u, float2x2((0.0f).xx, (0.0f).xx));
  v_13(160u, float2x3((0.0f).xxx, (0.0f).xxx));
  v_12(192u, float2x4((0.0f).xxxx, (0.0f).xxxx));
  v_11(224u, float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_10(256u, float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_9(304u, float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  v_8(352u, float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_7(384u, float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_6(448u, float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  float3 v_15[2] = (float3[2])0;
  v_4(512u, v_15);
  Inner v_16 = (Inner)0;
  v(544u, v_16);
  Inner v_17[4] = (Inner[4])0;
  v_1(552u, v_17);
}

