
Texture1D<float4> tint_resource_table_array[] : register(t51, space43);
Texture2D<int4> tint_resource_table_array_1[] : register(t51, space44);
Texture3D<uint4> tint_resource_table_array_2[] : register(t51, space45);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
void fs() {
  bool v = false;
  if ((0u < tint_resource_table_metadata.Load(0u))) {
    uint2 v_1 = uint2((tint_resource_table_metadata.Load(4u)).xx);
    v = any((v_1 == uint2(2u, 1u)));
  } else {
    v = false;
  }
  uint v_2 = 0u;
  if (v) {
    v_2 = 0u;
  } else {
    v_2 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint v_3 = v_2;
  float4 t1d = tint_resource_table_array[v_3].Load(int2(int(0), int(0)));
  uint v_4 = uint(int(1));
  bool v_5 = false;
  if ((v_4 < tint_resource_table_metadata.Load(0u))) {
    v_5 = (tint_resource_table_metadata.Load((4u + (v_4 * 4u))) == 7u);
  } else {
    v_5 = false;
  }
  uint v_6 = 0u;
  if (v_5) {
    v_6 = v_4;
  } else {
    v_6 = (1u + tint_resource_table_metadata.Load(0u));
  }
  uint v_7 = v_6;
  int4 t2d = tint_resource_table_array_1[v_7].Load(int3(int2(int(0), int(1)), int(0)));
  uint v_8 = uint(int(2));
  bool v_9 = false;
  if ((v_8 < tint_resource_table_metadata.Load(0u))) {
    v_9 = (tint_resource_table_metadata.Load((4u + (v_8 * 4u))) == 16u);
  } else {
    v_9 = false;
  }
  uint v_10 = 0u;
  if (v_9) {
    v_10 = v_8;
  } else {
    v_10 = (2u + tint_resource_table_metadata.Load(0u));
  }
  uint v_11 = v_10;
  uint4 tcube = tint_resource_table_array_2[v_11].Load(int4(int3(int(2), int(1), int(0)), int(0)));
}

