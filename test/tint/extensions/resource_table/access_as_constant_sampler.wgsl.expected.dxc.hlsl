struct fs_outputs {
  float4 tint_symbol : SV_Target0;
};


Texture2D<float4> tint_resource_table_array[] : register(t51, space43);
Texture2D tint_resource_table_array_1[] : register(t51, space45);
SamplerState tint_resource_table_array_2[] : register(s51, space46);
ByteAddressBuffer tint_resource_table_metadata : register(t52, space42);
float4 fs_inner() {
  uint v = uint(int(0));
  uint v_1 = uint(int(1));
  bool v_2 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    uint3 v_3 = uint3((tint_resource_table_metadata.Load((4u + (v * 4u)))).xxx);
    v_2 = any((v_3 == uint3(6u, 7u, 34u)));
  } else {
    v_2 = false;
  }
  bool has_resource = v_2;
  uint v_4 = 0u;
  if (has_resource) {
    v_4 = tint_resource_table_metadata.Load((4u + (v * 4u)));
  } else {
    v_4 = 6u;
  }
  uint texture_kind = v_4;
  uint v_5 = 0u;
  if (has_resource) {
    v_5 = v;
  } else {
    v_5 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_5;
  bool v_6 = false;
  if ((v_1 < tint_resource_table_metadata.Load(0u))) {
    uint2 v_7 = uint2((tint_resource_table_metadata.Load((4u + (v_1 * 4u)))).xx);
    v_6 = any((v_7 == uint2(40u, 41u)));
  } else {
    v_6 = false;
  }
  bool has_resource_1 = v_6;
  uint v_8 = 0u;
  if (has_resource_1) {
    v_8 = tint_resource_table_metadata.Load((4u + (v_1 * 4u)));
  } else {
    v_8 = 41u;
  }
  uint sampler_kind = v_8;
  uint v_9 = 0u;
  if (has_resource_1) {
    v_9 = v_1;
  } else {
    v_9 = (4u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx_1 = v_9;
  bool v_10 = false;
  if ((sampler_kind == 40u)) {
    v_10 = (texture_kind == 6u);
  } else {
    v_10 = true;
  }
  float4 v_11 = (0.0f).xxxx;
  if (v_10) {
    v_11 = tint_resource_table_array[item_idx].Sample(tint_resource_table_array_2[item_idx_1], (0.0f).xx);
  } else {
    v_11 = (0.0f).xxxx;
  }
  return v_11;
}

fs_outputs fs() {
  fs_outputs v_12 = {fs_inner()};
  return v_12;
}

