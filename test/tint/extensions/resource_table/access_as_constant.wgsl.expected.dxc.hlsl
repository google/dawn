
Texture1D<float4> tint_resource_table_array[] : register(t51, space43);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
void fs() {
  bool v = false;
  if ((2u < tint_resource_table_metadata.Load(0u))) {
    uint2 v_1 = uint2((tint_resource_table_metadata.Load(12u)).xx);
    v = any((v_1 == uint2(1u, 2u)));
  } else {
    v = false;
  }
  uint v_2 = 0u;
  if (v) {
    v_2 = 2u;
  } else {
    v_2 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_2;
  float4 texture_load = tint_resource_table_array[item_idx].Load(int2(int(0), int(0)));
}

