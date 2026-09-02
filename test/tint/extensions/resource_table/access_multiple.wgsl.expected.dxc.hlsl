
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
  uint v_2 = uint(int(1));
  bool v_3 = false;
  if ((v_2 < tint_resource_table_metadata.Load(0u))) {
    v_3 = (tint_resource_table_metadata.Load(8u) == 9u);
  } else {
    v_3 = false;
  }
  uint v_4 = 0u;
  if (v_3) {
    v_4 = v_2;
  } else {
    v_4 = (2u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx_1 = v_4;
  int4 t2d = tint_resource_table_array_1[item_idx_1].Load(int3(int(0), int(1), int(0)));
  uint v_5 = uint(int(2));
  bool v_6 = false;
  if ((v_5 < tint_resource_table_metadata.Load(0u))) {
    v_6 = (tint_resource_table_metadata.Load(12u) == 20u);
  } else {
    v_6 = false;
  }
  uint v_7 = 0u;
  if (v_6) {
    v_7 = v_5;
  } else {
    v_7 = (3u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx_2 = v_7;
  uint4 tcube = tint_resource_table_array_2[item_idx_2].Load(int4(int(2), int(1), int(0), int(0)));
}

