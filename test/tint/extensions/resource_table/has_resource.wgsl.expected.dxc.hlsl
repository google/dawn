
Texture2D<int4> tint_resource_table_array[] : register(t51, space43);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
void fs() {
  bool v = false;
  if ((4u < tint_resource_table_metadata.Load(0u))) {
    v = (tint_resource_table_metadata.Load(20u) == 7u);
  } else {
    v = false;
  }
  bool t = v;
}

