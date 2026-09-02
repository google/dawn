
RWByteAddressBuffer o : register(u0);
SamplerState tint_resource_table_array[] : register(s28, space4);
SamplerComparisonState tint_resource_table_array_1[] : register(s28, space6);
ByteAddressBuffer tint_resource_table_metadata : register(t29, space5);
void fs() {
  uint v = uint(int(0));
  bool v_1 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    v_1 = any((uint2((tint_resource_table_metadata.Load(4u)).xx) == uint2(40u, 41u)));
  } else {
    v_1 = false;
  }
  if (v_1) {
    o.Store(0u, (o.Load(0u) + 1u));
  }
  uint v_2 = uint(int(1));
  bool v_3 = false;
  if ((v_2 < tint_resource_table_metadata.Load(0u))) {
    v_3 = (tint_resource_table_metadata.Load(8u) == 42u);
  } else {
    v_3 = false;
  }
  if (v_3) {
    o.Store(0u, (o.Load(0u) + 1u));
  }
}

