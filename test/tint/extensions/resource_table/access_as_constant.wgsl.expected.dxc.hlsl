
Texture1D<float4> tint_resource_table_array[] : register(t51, space43);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
void fs() {
  bool v = false;
  if ((2u < tint_resource_table_metadata.Load(0u))) {
    v = (tint_resource_table_metadata.Load(12u) == 1u);
  } else {
    v = false;
  }
  uint v_1 = 0u;
  if (v) {
    v_1 = 2u;
  } else {
    v_1 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint v_2 = v_1;
  float4 texture_load = tint_resource_table_array[v_2].Load(int2(int(0), int(0)));
}

