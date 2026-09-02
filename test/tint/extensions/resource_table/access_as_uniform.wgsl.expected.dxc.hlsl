
cbuffer cbuffer_index : register(b0, space1) {
  uint4 index[1];
};
Texture3D<float4> tint_resource_table_array[] : register(t28, space4);
ByteAddressBuffer tint_resource_table_metadata : register(t29, space5);
void fs() {
  uint v = index[0u].x;
  bool v_1 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    v_1 = any((uint2((tint_resource_table_metadata.Load((4u + (v * 4u)))).xx) == uint2(16u, 17u)));
  } else {
    v_1 = false;
  }
  uint v_2 = 0u;
  if (v_1) {
    v_2 = v;
  } else {
    v_2 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_2;
  float4 texture_load = tint_resource_table_array[item_idx].Load((int(0)).xxxx);
}

