struct fs_outputs {
  float4 tint_symbol : SV_Target0;
};


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
  uint texture_kind = v_1;
  uint v_2 = 0u;
  if (has_resource) {
    v_2 = 0u;
  } else {
    v_2 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_2;
  bool v_3 = false;
  if ((1u < tint_resource_table_metadata.Load(0u))) {
    v_3 = any((uint2((tint_resource_table_metadata.Load(8u)).xx) == uint2(40u, 41u)));
  } else {
    v_3 = false;
  }
  bool has_resource_1 = v_3;
  uint v_4 = 0u;
  if (has_resource_1) {
    v_4 = tint_resource_table_metadata.Load(8u);
  } else {
    v_4 = 41u;
  }
  uint sampler_kind = v_4;
  uint v_5 = 0u;
  if (has_resource_1) {
    v_5 = 1u;
  } else {
    v_5 = (4u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx_1 = v_5;
  bool v_6 = false;
  if ((sampler_kind == 40u)) {
    v_6 = (texture_kind == 6u);
  } else {
    v_6 = true;
  }
  float4 v_7 = (0.0f).xxxx;
  if (v_6) {
    v_7 = tint_resource_table_array[item_idx].Sample(tint_resource_table_array_2[item_idx_1], (0.0f).xx);
  } else {
    v_7 = (0.0f).xxxx;
  }
  return v_7;
}

fs_outputs fs() {
  fs_outputs v_8 = {fs_inner()};
  return v_8;
}

