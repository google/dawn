struct main_inputs {
  uint idx : SV_GroupIndex;
};


RWByteAddressBuffer sb : register(u0);
void v(uint offset, matrix<float16_t, 4, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  sb.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
  sb.Store<vector<float16_t, 2> >((offset + 12u), obj[3u]);
}

void v_1(uint offset, matrix<float16_t, 4, 2> obj[2]) {
  {
    uint v_2 = 0u;
    v_2 = 0u;
    while(true) {
      uint v_3 = v_2;
      if ((v_3 >= 2u)) {
        break;
      }
      v((offset + (v_3 * 16u)), obj[v_3]);
      {
        v_2 = (v_3 + 1u);
      }
      continue;
    }
  }
}

void v_4(uint offset, float3 obj[2]) {
  {
    uint v_5 = 0u;
    v_5 = 0u;
    while(true) {
      uint v_6 = v_5;
      if ((v_6 >= 2u)) {
        break;
      }
      sb.Store3((offset + (v_6 * 16u)), asuint(obj[v_6]));
      {
        v_5 = (v_6 + 1u);
      }
      continue;
    }
  }
}

void v_7(uint offset, matrix<float16_t, 4, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
  sb.Store<vector<float16_t, 4> >((offset + 24u), obj[3u]);
}

void v_8(uint offset, matrix<float16_t, 4, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
  sb.Store<vector<float16_t, 3> >((offset + 24u), obj[3u]);
}

void v_9(uint offset, matrix<float16_t, 3, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
}

void v_10(uint offset, matrix<float16_t, 3, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
}

void v_11(uint offset, matrix<float16_t, 3, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  sb.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
}

void v_12(uint offset, matrix<float16_t, 2, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

void v_13(uint offset, matrix<float16_t, 2, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
}

void v_14(uint offset, matrix<float16_t, 2, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
}

void v_15(uint offset, float4x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
  sb.Store4((offset + 48u), asuint(obj[3u]));
}

void v_16(uint offset, float4x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
  sb.Store3((offset + 48u), asuint(obj[3u]));
}

void v_17(uint offset, float4x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
  sb.Store2((offset + 24u), asuint(obj[3u]));
}

void v_18(uint offset, float3x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
}

void v_19(uint offset, float3x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
}

void v_20(uint offset, float3x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
}

void v_21(uint offset, float2x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
}

void v_22(uint offset, float2x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
}

void v_23(uint offset, float2x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
}

void main_inner(uint idx) {
  sb.Store((0u + (idx * 800u)), asuint(0.0f));
  sb.Store((4u + (idx * 800u)), asuint(int(0)));
  sb.Store((8u + (idx * 800u)), 0u);
  sb.Store<float16_t>((12u + (idx * 800u)), float16_t(0.0h));
  sb.Store2((16u + (idx * 800u)), asuint((0.0f).xx));
  sb.Store2((24u + (idx * 800u)), asuint(int2((int(0)).xx)));
  sb.Store2((32u + (idx * 800u)), (0u).xx);
  sb.Store<vector<float16_t, 2> >((40u + (idx * 800u)), (float16_t(0.0h)).xx);
  sb.Store3((48u + (idx * 800u)), asuint((0.0f).xxx));
  sb.Store3((64u + (idx * 800u)), asuint(int3((int(0)).xxx)));
  sb.Store3((80u + (idx * 800u)), (0u).xxx);
  sb.Store<vector<float16_t, 3> >((96u + (idx * 800u)), (float16_t(0.0h)).xxx);
  sb.Store4((112u + (idx * 800u)), asuint((0.0f).xxxx));
  sb.Store4((128u + (idx * 800u)), asuint(int4((int(0)).xxxx)));
  sb.Store4((144u + (idx * 800u)), (0u).xxxx);
  sb.Store<vector<float16_t, 4> >((160u + (idx * 800u)), (float16_t(0.0h)).xxxx);
  v_23((168u + (idx * 800u)), float2x2((0.0f).xx, (0.0f).xx));
  v_22((192u + (idx * 800u)), float2x3((0.0f).xxx, (0.0f).xxx));
  v_21((224u + (idx * 800u)), float2x4((0.0f).xxxx, (0.0f).xxxx));
  v_20((256u + (idx * 800u)), float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_19((288u + (idx * 800u)), float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_18((336u + (idx * 800u)), float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  v_17((384u + (idx * 800u)), float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  v_16((416u + (idx * 800u)), float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  v_15((480u + (idx * 800u)), float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  v_14((544u + (idx * 800u)), matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  v_13((552u + (idx * 800u)), matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  v_12((568u + (idx * 800u)), matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  v_11((584u + (idx * 800u)), matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  v_10((600u + (idx * 800u)), matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  v_9((624u + (idx * 800u)), matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  v((648u + (idx * 800u)), matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  v_8((664u + (idx * 800u)), matrix<float16_t, 4, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  v_7((696u + (idx * 800u)), matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  float3 v_24[2] = (float3[2])0;
  v_4((736u + (idx * 800u)), v_24);
  matrix<float16_t, 4, 2> v_25[2] = (matrix<float16_t, 4, 2>[2])0;
  v_1((768u + (idx * 800u)), v_25);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

