struct main_inputs {
  uint idx : SV_GroupIndex;
};


RWByteAddressBuffer sb : register(u0);
void v(uint offset, float3 obj[2]) {
  {
    uint v_1 = 0u;
    v_1 = 0u;
    while(true) {
      uint v_2 = v_1;
      if ((v_2 >= 2u)) {
        break;
      }
      sb.Store3((offset + (v_2 * 16u)), asuint(obj[v_2]));
      {
        v_1 = (v_2 + 1u);
      }
      continue;
    }
  }
}

void v_3(uint offset, float4x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
  sb.Store4((offset + 48u), asuint(obj[3u]));
}

void v_4(uint offset, float4x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
  sb.Store3((offset + 48u), asuint(obj[3u]));
}

void v_5(uint offset, float4x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
  sb.Store2((offset + 24u), asuint(obj[3u]));
}

void v_6(uint offset, float3x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
}

void v_7(uint offset, float3x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
}

void v_8(uint offset, float3x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
}

void v_9(uint offset, float2x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
}

void v_10(uint offset, float2x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
}

void v_11(uint offset, float2x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
}

void main_inner(uint idx) {
  uint v_12 = (0u + (uint(idx) * 544u));
  sb.Store(v_12, asuint(0.0f));
  uint v_13 = (4u + (uint(idx) * 544u));
  sb.Store(v_13, asuint(int(0)));
  sb.Store((8u + (uint(idx) * 544u)), 0u);
  uint v_14 = (16u + (uint(idx) * 544u));
  sb.Store2(v_14, asuint((0.0f).xx));
  uint v_15 = (24u + (uint(idx) * 544u));
  sb.Store2(v_15, asuint(int2((int(0)).xx)));
  sb.Store2((32u + (uint(idx) * 544u)), (0u).xx);
  uint v_16 = (48u + (uint(idx) * 544u));
  sb.Store3(v_16, asuint((0.0f).xxx));
  uint v_17 = (64u + (uint(idx) * 544u));
  sb.Store3(v_17, asuint(int3((int(0)).xxx)));
  sb.Store3((80u + (uint(idx) * 544u)), (0u).xxx);
  uint v_18 = (96u + (uint(idx) * 544u));
  sb.Store4(v_18, asuint((0.0f).xxxx));
  uint v_19 = (112u + (uint(idx) * 544u));
  sb.Store4(v_19, asuint(int4((int(0)).xxxx)));
  sb.Store4((128u + (uint(idx) * 544u)), (0u).xxxx);
  v_11((144u + (uint(idx) * 544u)), float2x2((0.0f).xx, (0.0f).xx));
  v_10((160u + (uint(idx) * 544u)), float2x3((0.0f).xxx, (0.0f).xxx));
  v_9((192u + (uint(idx) * 544u)), float2x4((0.0f).xxxx, (0.0f).xxxx));
  v_8((224u + (uint(idx) * 544u)), float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_7((256u + (uint(idx) * 544u)), float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_6((304u + (uint(idx) * 544u)), float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  v_5((352u + (uint(idx) * 544u)), float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_4((384u + (uint(idx) * 544u)), float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_3((448u + (uint(idx) * 544u)), float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  float3 v_20[2] = (float3[2])0;
  v((512u + (uint(idx) * 544u)), v_20);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

