struct fs_outputs {
  float4 tint_symbol : SV_Target0;
};


Texture2D<float4> tint_resource_table_array[] : register(t51, space43);
SamplerState tint_resource_table_array_1[] : register(s51, space44);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
float4 fs_inner() {
  bool v = false;
  if ((2u < tint_resource_table_metadata.Load(0u))) {
    v = (tint_resource_table_metadata.Load(12u) == 5u);
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
  bool v_3 = false;
  if ((3u < tint_resource_table_metadata.Load(0u))) {
    v_3 = (tint_resource_table_metadata.Load(16u) == 33u);
  } else {
    v_3 = false;
  }
  uint v_4 = 0u;
  if (v_3) {
    v_4 = 3u;
  } else {
    v_4 = (1u + tint_resource_table_metadata.Load(0u));
  }
  uint v_5 = v_4;
  return tint_resource_table_array[v_2].Sample(tint_resource_table_array_1[v_5], (0.0f).xx);
}

fs_outputs fs() {
  fs_outputs v_6 = {fs_inner()};
  return v_6;
}

