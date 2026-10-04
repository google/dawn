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
    uint idx = 0u;
    while(true) {
      uint v_2 = idx;
      if ((v_2 >= 2u)) {
        break;
      }
      v((offset + (v_2 * 16u)), obj[v_2]);
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_3(uint offset, float3 obj[2]) {
  {
    uint idx = 0u;
    while(true) {
      uint v_4 = idx;
      if ((v_4 >= 2u)) {
        break;
      }
      sb.Store3((offset + (v_4 * 16u)), asuint(obj[v_4]));
      {
        idx = (idx + 1u);
      }
    }
  }
}

void v_5(uint offset, matrix<float16_t, 4, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
  sb.Store<vector<float16_t, 4> >((offset + 24u), obj[3u]);
}

void v_6(uint offset, matrix<float16_t, 4, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
  sb.Store<vector<float16_t, 3> >((offset + 24u), obj[3u]);
}

void v_7(uint offset, matrix<float16_t, 3, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
}

void v_8(uint offset, matrix<float16_t, 3, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  sb.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
}

void v_9(uint offset, matrix<float16_t, 3, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
  sb.Store<vector<float16_t, 2> >((offset + 8u), obj[2u]);
}

void v_10(uint offset, matrix<float16_t, 2, 4> obj) {
  sb.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

void v_11(uint offset, matrix<float16_t, 2, 3> obj) {
  sb.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
}

void v_12(uint offset, matrix<float16_t, 2, 2> obj) {
  sb.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  sb.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
}

void v_13(uint offset, float4x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
  sb.Store4((offset + 48u), asuint(obj[3u]));
}

void v_14(uint offset, float4x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
  sb.Store3((offset + 48u), asuint(obj[3u]));
}

void v_15(uint offset, float4x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
  sb.Store2((offset + 24u), asuint(obj[3u]));
}

void v_16(uint offset, float3x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
  sb.Store4((offset + 32u), asuint(obj[2u]));
}

void v_17(uint offset, float3x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
  sb.Store3((offset + 32u), asuint(obj[2u]));
}

void v_18(uint offset, float3x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
  sb.Store2((offset + 16u), asuint(obj[2u]));
}

void v_19(uint offset, float2x4 obj) {
  sb.Store4((offset + 0u), asuint(obj[0u]));
  sb.Store4((offset + 16u), asuint(obj[1u]));
}

void v_20(uint offset, float2x3 obj) {
  sb.Store3((offset + 0u), asuint(obj[0u]));
  sb.Store3((offset + 16u), asuint(obj[1u]));
}

void v_21(uint offset, float2x2 obj) {
  sb.Store2((offset + 0u), asuint(obj[0u]));
  sb.Store2((offset + 8u), asuint(obj[1u]));
}

void main_inner(uint idx) {
  uint v_22 = 0u;
  sb.GetDimensions(v_22);
  sb.Store((0u + (min(idx, ((v_22 / 800u) - 1u)) * 800u)), 0u);
  uint v_23 = 0u;
  sb.GetDimensions(v_23);
  sb.Store((4u + (min(idx, ((v_23 / 800u) - 1u)) * 800u)), 0u);
  uint v_24 = 0u;
  sb.GetDimensions(v_24);
  sb.Store((8u + (min(idx, ((v_24 / 800u) - 1u)) * 800u)), 0u);
  uint v_25 = 0u;
  sb.GetDimensions(v_25);
  sb.Store<float16_t>((12u + (min(idx, ((v_25 / 800u) - 1u)) * 800u)), float16_t(0.0h));
  uint v_26 = 0u;
  sb.GetDimensions(v_26);
  sb.Store2((16u + (min(idx, ((v_26 / 800u) - 1u)) * 800u)), (0u).xx);
  uint v_27 = 0u;
  sb.GetDimensions(v_27);
  sb.Store2((24u + (min(idx, ((v_27 / 800u) - 1u)) * 800u)), (0u).xx);
  uint v_28 = 0u;
  sb.GetDimensions(v_28);
  sb.Store2((32u + (min(idx, ((v_28 / 800u) - 1u)) * 800u)), (0u).xx);
  uint v_29 = 0u;
  sb.GetDimensions(v_29);
  sb.Store<vector<float16_t, 2> >((40u + (min(idx, ((v_29 / 800u) - 1u)) * 800u)), (float16_t(0.0h)).xx);
  uint v_30 = 0u;
  sb.GetDimensions(v_30);
  sb.Store3((48u + (min(idx, ((v_30 / 800u) - 1u)) * 800u)), (0u).xxx);
  uint v_31 = 0u;
  sb.GetDimensions(v_31);
  sb.Store3((64u + (min(idx, ((v_31 / 800u) - 1u)) * 800u)), (0u).xxx);
  uint v_32 = 0u;
  sb.GetDimensions(v_32);
  sb.Store3((80u + (min(idx, ((v_32 / 800u) - 1u)) * 800u)), (0u).xxx);
  uint v_33 = 0u;
  sb.GetDimensions(v_33);
  sb.Store<vector<float16_t, 3> >((96u + (min(idx, ((v_33 / 800u) - 1u)) * 800u)), (float16_t(0.0h)).xxx);
  uint v_34 = 0u;
  sb.GetDimensions(v_34);
  sb.Store4((112u + (min(idx, ((v_34 / 800u) - 1u)) * 800u)), (0u).xxxx);
  uint v_35 = 0u;
  sb.GetDimensions(v_35);
  sb.Store4((128u + (min(idx, ((v_35 / 800u) - 1u)) * 800u)), (0u).xxxx);
  uint v_36 = 0u;
  sb.GetDimensions(v_36);
  sb.Store4((144u + (min(idx, ((v_36 / 800u) - 1u)) * 800u)), (0u).xxxx);
  uint v_37 = 0u;
  sb.GetDimensions(v_37);
  sb.Store<vector<float16_t, 4> >((160u + (min(idx, ((v_37 / 800u) - 1u)) * 800u)), (float16_t(0.0h)).xxxx);
  uint v_38 = 0u;
  sb.GetDimensions(v_38);
  v_21((168u + (min(idx, ((v_38 / 800u) - 1u)) * 800u)), float2x2((0.0f).xx, (0.0f).xx));
  uint v_39 = 0u;
  sb.GetDimensions(v_39);
  v_20((192u + (min(idx, ((v_39 / 800u) - 1u)) * 800u)), float2x3((0.0f).xxx, (0.0f).xxx));
  uint v_40 = 0u;
  sb.GetDimensions(v_40);
  v_19((224u + (min(idx, ((v_40 / 800u) - 1u)) * 800u)), float2x4((0.0f).xxxx, (0.0f).xxxx));
  uint v_41 = 0u;
  sb.GetDimensions(v_41);
  v_18((256u + (min(idx, ((v_41 / 800u) - 1u)) * 800u)), float3x2((0.0f).xx, (0.0f).xx, (0.0f).xx));
  uint v_42 = 0u;
  sb.GetDimensions(v_42);
  v_17((288u + (min(idx, ((v_42 / 800u) - 1u)) * 800u)), float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  uint v_43 = 0u;
  sb.GetDimensions(v_43);
  v_16((336u + (min(idx, ((v_43 / 800u) - 1u)) * 800u)), float3x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  uint v_44 = 0u;
  sb.GetDimensions(v_44);
  v_15((384u + (min(idx, ((v_44 / 800u) - 1u)) * 800u)), float4x2((0.0f).xx, (0.0f).xx, (0.0f).xx, (0.0f).xx));
  uint v_45 = 0u;
  sb.GetDimensions(v_45);
  v_14((416u + (min(idx, ((v_45 / 800u) - 1u)) * 800u)), float4x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx, (0.0f).xxx));
  uint v_46 = 0u;
  sb.GetDimensions(v_46);
  v_13((480u + (min(idx, ((v_46 / 800u) - 1u)) * 800u)), float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx));
  uint v_47 = 0u;
  sb.GetDimensions(v_47);
  v_12((544u + (min(idx, ((v_47 / 800u) - 1u)) * 800u)), matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  uint v_48 = 0u;
  sb.GetDimensions(v_48);
  v_11((552u + (min(idx, ((v_48 / 800u) - 1u)) * 800u)), matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  uint v_49 = 0u;
  sb.GetDimensions(v_49);
  v_10((568u + (min(idx, ((v_49 / 800u) - 1u)) * 800u)), matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  uint v_50 = 0u;
  sb.GetDimensions(v_50);
  v_9((584u + (min(idx, ((v_50 / 800u) - 1u)) * 800u)), matrix<float16_t, 3, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  uint v_51 = 0u;
  sb.GetDimensions(v_51);
  v_8((600u + (min(idx, ((v_51 / 800u) - 1u)) * 800u)), matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  uint v_52 = 0u;
  sb.GetDimensions(v_52);
  v_7((624u + (min(idx, ((v_52 / 800u) - 1u)) * 800u)), matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  uint v_53 = 0u;
  sb.GetDimensions(v_53);
  v((648u + (min(idx, ((v_53 / 800u) - 1u)) * 800u)), matrix<float16_t, 4, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx, (float16_t(0.0h)).xx));
  uint v_54 = 0u;
  sb.GetDimensions(v_54);
  v_6((664u + (min(idx, ((v_54 / 800u) - 1u)) * 800u)), matrix<float16_t, 4, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx));
  uint v_55 = 0u;
  sb.GetDimensions(v_55);
  v_5((696u + (min(idx, ((v_55 / 800u) - 1u)) * 800u)), matrix<float16_t, 4, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx));
  uint v_56 = 0u;
  sb.GetDimensions(v_56);
  float3 v_57[2] = (float3[2])0;
  v_3((736u + (min(idx, ((v_56 / 800u) - 1u)) * 800u)), v_57);
  uint v_58 = 0u;
  sb.GetDimensions(v_58);
  matrix<float16_t, 4, 2> v_59[2] = (matrix<float16_t, 4, 2>[2])0;
  v_1((768u + (min(idx, ((v_58 / 800u) - 1u)) * 800u)), v_59);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.idx);
}

