
Texture2D<int4> tint_resource_table_array[] : register(t28, space4);
ByteAddressBuffer tint_resource_table_metadata : register(t29, space5);
void fs() {
  bool v = false;
  if ((4u < tint_resource_table_metadata.Load(0u))) {
    v = (tint_resource_table_metadata.Load(20u) == 9u);
  } else {
    v = false;
  }
  bool t = v;
}

