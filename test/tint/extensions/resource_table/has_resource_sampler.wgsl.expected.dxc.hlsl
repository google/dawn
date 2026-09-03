
RWByteAddressBuffer o : register(u0);
SamplerState tint_resource_table_array[] : register(s28, space4);
SamplerComparisonState tint_resource_table_array_1[] : register(s28, space6);
ByteAddressBuffer tint_resource_table_metadata : register(t29, space5);
void fs() {
  bool v = false;
  if ((0u < tint_resource_table_metadata.Load(0u))) {
    v = any((uint2((tint_resource_table_metadata.Load(4u)).xx) == uint2(40u, 41u)));
  } else {
    v = false;
  }
  if (v) {
    o.Store(0u, (o.Load(0u) + 1u));
  }
  bool v_1 = false;
  if ((1u < tint_resource_table_metadata.Load(0u))) {
    v_1 = (tint_resource_table_metadata.Load(8u) == 42u);
  } else {
    v_1 = false;
  }
  if (v_1) {
    o.Store(0u, (o.Load(0u) + 1u));
  }
}

