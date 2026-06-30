
cbuffer cbuffer_index : register(b0, space1) {
  uint4 index[1];
};
Texture3D<float4> tint_resource_table_array[] : register(t51, space43);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
void fs() {
  uint v = index[0u].x;
  bool v_1 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    uint2 v_2 = uint2((tint_resource_table_metadata.Load((4u + (v * 4u)))).xx);
    v_1 = any((v_2 == uint2(16u, 17u)));
  } else {
    v_1 = false;
  }
  uint v_3 = 0u;
  if (v_1) {
    v_3 = v;
  } else {
    v_3 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_3;
  float4 texture_load = tint_resource_table_array[item_idx].Load(int4((int(0)).xxx, int(0)));
}

