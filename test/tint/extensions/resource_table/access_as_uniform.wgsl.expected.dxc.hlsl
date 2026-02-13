
cbuffer cbuffer_index : register(b0, space1) {
  uint4 index[1];
};
Texture3D<float4> tint_resource_table_array[] : register(t51, space43);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
void fs() {
  uint v = index[0u].x;
  bool v_1 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    v_1 = (tint_resource_table_metadata.Load((4u + (v * 4u))) == 13u);
  } else {
    v_1 = false;
  }
  uint v_2 = 0u;
  if (v_1) {
    v_2 = v;
  } else {
    v_2 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint v_3 = v_2;
  float4 texture_load = tint_resource_table_array[v_3].Load(int4((int(0)).xxx, int(0)));
}

