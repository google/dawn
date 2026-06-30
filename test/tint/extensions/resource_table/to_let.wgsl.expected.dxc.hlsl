struct fs_outputs {
  float4 tint_symbol : SV_Target0;
};


Texture2D<float4> tint_resource_table_array[] : register(t51, space43);
Texture2D tint_resource_table_array_1[] : register(t51, space45);
SamplerState tint_resource_table_array_2[] : register(s51, space46);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
float4 fs_inner() {
  bool v = false;
  if ((2u < tint_resource_table_metadata.Load(0u))) {
    uint3 v_1 = uint3((tint_resource_table_metadata.Load(12u)).xxx);
    v = any((v_1 == uint3(6u, 7u, 34u)));
  } else {
    v = false;
  }
  bool has_resource = v;
  uint v_2 = 0u;
  if (has_resource) {
    v_2 = tint_resource_table_metadata.Load(12u);
  } else {
    v_2 = 6u;
  }
  uint texture_kind = v_2;
  uint v_3 = 0u;
  if (has_resource) {
    v_3 = 2u;
  } else {
    v_3 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_3;
  bool v_4 = false;
  if ((3u < tint_resource_table_metadata.Load(0u))) {
    uint2 v_5 = uint2((tint_resource_table_metadata.Load(16u)).xx);
    v_4 = any((v_5 == uint2(40u, 41u)));
  } else {
    v_4 = false;
  }
  bool has_resource_1 = v_4;
  uint v_6 = 0u;
  if (has_resource_1) {
    v_6 = tint_resource_table_metadata.Load(16u);
  } else {
    v_6 = 41u;
  }
  uint sampler_kind = v_6;
  uint v_7 = 0u;
  if (has_resource_1) {
    v_7 = 3u;
  } else {
    v_7 = (4u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx_1 = v_7;
  bool v_8 = false;
  if ((sampler_kind == 40u)) {
    v_8 = (texture_kind == 6u);
  } else {
    v_8 = true;
  }
  float4 v_9 = (0.0f).xxxx;
  if (v_8) {
    v_9 = tint_resource_table_array[item_idx].Sample(tint_resource_table_array_2[item_idx_1], (0.0f).xx);
  } else {
    v_9 = (0.0f).xxxx;
  }
  return v_9;
}

fs_outputs fs() {
  fs_outputs v_10 = {fs_inner()};
  return v_10;
}

