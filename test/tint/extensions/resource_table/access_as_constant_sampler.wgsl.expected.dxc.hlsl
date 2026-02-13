struct fs_outputs {
  float4 tint_symbol : SV_Target0;
};


Texture2D<float4> tint_resource_table_array[] : register(t51, space43);
SamplerState tint_resource_table_array_1[] : register(s51, space44);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
float4 fs_inner() {
  uint v = uint(int(0));
  bool v_1 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    v_1 = (tint_resource_table_metadata.Load((4u + (v * 4u))) == 5u);
  } else {
    v_1 = false;
  }
  uint v_2 = 0u;
  if (v_1) {
    v_2 = v;
  } else {
    v_2 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint v_3 = v_2;
  uint v_4 = uint(int(1));
  bool v_5 = false;
  if ((v_4 < tint_resource_table_metadata.Load(0u))) {
    v_5 = (tint_resource_table_metadata.Load((4u + (v_4 * 4u))) == 33u);
  } else {
    v_5 = false;
  }
  uint v_6 = 0u;
  if (v_5) {
    v_6 = v_4;
  } else {
    v_6 = (1u + tint_resource_table_metadata.Load(0u));
  }
  uint v_7 = v_6;
  return tint_resource_table_array[v_3].Sample(tint_resource_table_array_1[v_7], (0.0f).xx);
}

fs_outputs fs() {
  fs_outputs v_8 = {fs_inner()};
  return v_8;
}

