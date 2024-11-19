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
  sb.Store((0u + (idx * 544u)), asuint(0.0f));
  sb.Store((4u + (idx * 544u)), asuint(int(0)));
  sb.Store((8u + (idx * 544u)), 0u);
  sb.Store2((16u + (idx * 544u)), asuint((0.0f).xx));
  sb.Store2((24u + (idx * 544u)), asuint(int2((int(0)).xx)));
  sb.Store2((32u + (idx * 544u)), (0u).xx);
  sb.Store3((48u + (idx * 544u)), asuint((0.0f).xxx));
  sb.Store3((64u + (idx * 544u)), asuint(int3((int(0)).xxx)));
  sb.Store3((80u + (idx * 544u)), (0u).xxx);
  sb.Store4((96u + (idx * 544u)), asuint((0.0f).xxxx));
  sb.Store4((112u + (idx * 544u)), asuint(int4((int(0)).xxxx)));
  sb.Store4((128u + (idx * 544u)), (0u).xxxx);
  v_11((144u + (idx * 544u)), float2x2((0.0f).xx, (0.0f).xx));
  v_10((160u + (idx * 544u)), float2x3((0.0f).xxx, (0.0f).xxx));
  v_9((192u + (idx * 544u)), float2x4((0.0f).xxxx, (0.0f).xxxx));
  v_8((224u + (idx * 544u)), float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_7((256u + (idx * 544u)), float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_6((304u + (idx * 544u)), float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  v_5((352u + (idx * 544u)), float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_4((384u + (idx * 544u)), float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_3((448u + (idx * 544u)), float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  float3 v_12[2] = (float3[2])0;
  v((512u + (idx * 544u)), v_12);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

