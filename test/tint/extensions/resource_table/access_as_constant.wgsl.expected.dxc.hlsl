
Texture1D<float4> tint_resource_table_array[] : register(t28, space4);
ByteAddressBuffer tint_resource_table_metadata : register(t29, space5);
void fs() {
  bool v = false;
  if ((2u < tint_resource_table_metadata.Load(0u))) {
    v = any((uint2((tint_resource_table_metadata.Load(12u)).xx) == uint2(1u, 2u)));
  } else {
    v = false;
  }
  uint v_1 = 0u;
  if (v) {
    v_1 = 2u;
  } else {
    v_1 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_1;
  float4 texture_load = tint_resource_table_array[item_idx].Load((int(0)).xx);
}

