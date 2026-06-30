struct fs_outputs {
  float4 tint_symbol : SV_Target0;
};


Texture2D<float4> t : register(t0);
Texture2D<float4> tint_resource_table_array[] : register(t51, space43);
Texture2D tint_resource_table_array_1[] : register(t51, space45);
SamplerState tint_resource_table_array_2[] : register(s51, space46);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
float4 fs_inner() {
  uint v = uint(int(0));
  bool v_1 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    uint2 v_2 = uint2((tint_resource_table_metadata.Load((4u + (v * 4u)))).xx);
    v_1 = any((v_2 == uint2(40u, 41u)));
  } else {
    v_1 = false;
  }
  bool has_resource = v_1;
  uint v_3 = 0u;
  if (has_resource) {
    v_3 = tint_resource_table_metadata.Load((4u + (v * 4u)));
  } else {
    v_3 = 41u;
  }
  uint v_4 = 0u;
  if (has_resource) {
    v_4 = v;
  } else {
    v_4 = (4u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_4;
  return t.Sample(tint_resource_table_array_2[item_idx], (0.0f).xx);
}

fs_outputs fs() {
  fs_outputs v_5 = {fs_inner()};
  return v_5;
}

