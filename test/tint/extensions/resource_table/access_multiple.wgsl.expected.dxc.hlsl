
Texture1D<float4> tint_resource_table_array[] : register(t51, space43);
Texture2D<int4> tint_resource_table_array_1[] : register(t51, space45);
Texture3D<uint4> tint_resource_table_array_2[] : register(t51, space46);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
void fs() {
  bool v = false;
  if ((0u < tint_resource_table_metadata.Load(0u))) {
    uint2 v_1 = uint2((tint_resource_table_metadata.Load(4u)).xx);
    v = any((v_1 == uint2(1u, 2u)));
  } else {
    v = false;
  }
  uint v_2 = 0u;
  if (v) {
    v_2 = 0u;
  } else {
    v_2 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_2;
  float4 t1d = tint_resource_table_array[item_idx].Load(int2(int(0), int(0)));
  uint v_3 = uint(int(1));
  bool v_4 = false;
  if ((v_3 < tint_resource_table_metadata.Load(0u))) {
    v_4 = (tint_resource_table_metadata.Load((4u + (v_3 * 4u))) == 9u);
  } else {
    v_4 = false;
  }
  uint v_5 = 0u;
  if (v_4) {
    v_5 = v_3;
  } else {
    v_5 = (2u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx_1 = v_5;
  int4 t2d = tint_resource_table_array_1[item_idx_1].Load(int3(int2(int(0), int(1)), int(0)));
  uint v_6 = uint(int(2));
  bool v_7 = false;
  if ((v_6 < tint_resource_table_metadata.Load(0u))) {
    v_7 = (tint_resource_table_metadata.Load((4u + (v_6 * 4u))) == 20u);
  } else {
    v_7 = false;
  }
  uint v_8 = 0u;
  if (v_7) {
    v_8 = v_6;
  } else {
    v_8 = (3u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx_2 = v_8;
  uint4 tcube = tint_resource_table_array_2[item_idx_2].Load(int4(int3(int(2), int(1), int(0)), int(0)));
}

