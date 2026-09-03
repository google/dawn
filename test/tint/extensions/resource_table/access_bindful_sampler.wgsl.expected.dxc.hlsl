struct fs_outputs {
  float4 tint_symbol : SV_Target0;
};


SamplerState s : register(s0);
Texture2D<float4> tint_resource_table_array[] : register(t28, space4);
Texture2D tint_resource_table_array_1[] : register(t28, space6);
SamplerState tint_resource_table_array_2[] : register(s28, space7);
ByteAddressBuffer tint_resource_table_metadata : register(t29, space5);
float4 fs_inner() {
  bool v = false;
  if ((0u < tint_resource_table_metadata.Load(0u))) {
    v = any((uint3((tint_resource_table_metadata.Load(4u)).xxx) == uint3(6u, 7u, 34u)));
  } else {
    v = false;
  }
  bool has_resource = v;
  uint v_1 = 0u;
  if (has_resource) {
    v_1 = tint_resource_table_metadata.Load(4u);
  } else {
    v_1 = 6u;
  }
  uint v_2 = 0u;
  if (has_resource) {
    v_2 = 0u;
  } else {
    v_2 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_2;
  return tint_resource_table_array[item_idx].Sample(s, (0.0f).xx);
}

fs_outputs fs() {
  fs_outputs v_3 = {fs_inner()};
  return v_3;
}

