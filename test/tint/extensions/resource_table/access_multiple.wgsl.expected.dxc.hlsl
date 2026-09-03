
Texture1D<float4> tint_resource_table_array[] : register(t28, space4);
Texture2D<int4> tint_resource_table_array_1[] : register(t28, space6);
Texture3D<uint4> tint_resource_table_array_2[] : register(t28, space7);
ByteAddressBuffer tint_resource_table_metadata : register(t29, space5);
void fs() {
  bool v = false;
  if ((0u < tint_resource_table_metadata.Load(0u))) {
    v = any((uint2((tint_resource_table_metadata.Load(4u)).xx) == uint2(1u, 2u)));
  } else {
    v = false;
  }
  uint v_1 = 0u;
  if (v) {
    v_1 = 0u;
  } else {
    v_1 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_1;
  float4 t1d = tint_resource_table_array[item_idx].Load((int(0)).xx);
  bool v_2 = false;
  if ((1u < tint_resource_table_metadata.Load(0u))) {
    v_2 = (tint_resource_table_metadata.Load(8u) == 9u);
  } else {
    v_2 = false;
  }
  uint v_3 = 0u;
  if (v_2) {
    v_3 = 1u;
  } else {
    v_3 = (2u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx_1 = v_3;
  int4 t2d = tint_resource_table_array_1[item_idx_1].Load(int3(int(0), int(1), int(0)));
  bool v_4 = false;
  if ((2u < tint_resource_table_metadata.Load(0u))) {
    v_4 = (tint_resource_table_metadata.Load(12u) == 20u);
  } else {
    v_4 = false;
  }
  uint v_5 = 0u;
  if (v_4) {
    v_5 = 2u;
  } else {
    v_5 = (3u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx_2 = v_5;
  uint4 tcube = tint_resource_table_array_2[item_idx_2].Load(int4(int(2), int(1), int(0), int(0)));
}

