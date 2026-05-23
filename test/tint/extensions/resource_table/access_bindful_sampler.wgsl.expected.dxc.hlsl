struct fs_outputs {
  float4 tint_symbol : SV_Target0;
};


SamplerState s : register(s0);
Texture2D<float4> tint_resource_table_array[] : register(t51, space43);
Texture2D tint_resource_table_array_1[] : register(t51, space45);
SamplerState tint_resource_table_array_2[] : register(s51, space46);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
float4 fs_inner() {
  uint v = uint(int(0));
  bool v_1 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    uint3 v_2 = uint3((tint_resource_table_metadata.Load((4u + (v * 4u)))).xxx);
    v_1 = any((v_2 == uint3(6u, 7u, 34u)));
  } else {
    v_1 = false;
  }
  bool v_3 = v_1;
  uint v_4 = 0u;
  if (v_3) {
    v_4 = tint_resource_table_metadata.Load((4u + (v * 4u)));
  } else {
    v_4 = 6u;
  }
  uint v_5 = 0u;
  if (v_3) {
    v_5 = v;
  } else {
    v_5 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint v_6 = v_5;
  return tint_resource_table_array[v_6].Sample(s, (0.0f).xx);
}

fs_outputs fs() {
  fs_outputs v_7 = {fs_inner()};
  return v_7;
}

