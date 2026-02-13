
RWByteAddressBuffer o : register(u0);
SamplerState tint_resource_table_array[] : register(s51, space43);
SamplerComparisonState tint_resource_table_array_1[] : register(s51, space44);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
void fs() {
  uint v = uint(int(0));
  bool v_1 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    v_1 = (tint_resource_table_metadata.Load((4u + (v * 4u))) == 34u);
  } else {
    v_1 = false;
  }
  if (v_1) {
    o.Store(0u, (o.Load(0u) + 1u));
  }
  uint v_2 = uint(int(1));
  bool v_3 = false;
  if ((v_2 < tint_resource_table_metadata.Load(0u))) {
    v_3 = (tint_resource_table_metadata.Load((4u + (v_2 * 4u))) == 35u);
  } else {
    v_3 = false;
  }
  if (v_3) {
    o.Store(0u, (o.Load(0u) + 1u));
  }
}

