
Texture1D<float4> tint_resource_table_array[] : register(t51, space43);
Texture2D<int4> tint_resource_table_array_1[] : register(t51, space44);
Texture3D<uint4> tint_resource_table_array_2[] : register(t51, space45);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
void fs() {
  bool v = false;
  if ((0u < tint_resource_table_metadata.Load(0u))) {
    v = (tint_resource_table_metadata.Load(4u) == 2u);
  } else {
    v = false;
  }
  uint v_1 = 0u;
  if (v) {
    v_1 = 0u;
  } else {
    v_1 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint v_2 = v_1;
  float4 t1d = tint_resource_table_array[v_2].Load(int2(int(0), int(0)));
  uint v_3 = uint(int(1));
  bool v_4 = false;
  if ((v_3 < tint_resource_table_metadata.Load(0u))) {
    v_4 = (tint_resource_table_metadata.Load((4u + (v_3 * 4u))) == 7u);
  } else {
    v_4 = false;
  }
  uint v_5 = 0u;
  if (v_4) {
    v_5 = v_3;
  } else {
    v_5 = (1u + tint_resource_table_metadata.Load(0u));
  }
  uint v_6 = v_5;
  int4 t2d = tint_resource_table_array_1[v_6].Load(int3(int2(int(0), int(1)), int(0)));
  uint v_7 = uint(int(2));
  bool v_8 = false;
  if ((v_7 < tint_resource_table_metadata.Load(0u))) {
    v_8 = (tint_resource_table_metadata.Load((4u + (v_7 * 4u))) == 16u);
  } else {
    v_8 = false;
  }
  uint v_9 = 0u;
  if (v_8) {
    v_9 = v_7;
  } else {
    v_9 = (2u + tint_resource_table_metadata.Load(0u));
  }
  uint v_10 = v_9;
  uint4 tcube = tint_resource_table_array_2[v_10].Load(int4(int3(int(2), int(1), int(0)), int(0)));
}

