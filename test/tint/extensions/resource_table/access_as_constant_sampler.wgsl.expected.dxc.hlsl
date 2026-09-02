struct fs_outputs {
  float4 tint_symbol : SV_Target0;
};


Texture2D<float4> tint_resource_table_array[] : register(t28, space4);
Texture2D tint_resource_table_array_1[] : register(t28, space6);
SamplerState tint_resource_table_array_2[] : register(s28, space7);
ByteAddressBuffer tint_resource_table_metadata : register(t29, space5);
float4 fs_inner() {
  uint v = uint(int(0));
  uint v_1 = uint(int(1));
  bool v_2 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    v_2 = any((uint3((tint_resource_table_metadata.Load(4u)).xxx) == uint3(6u, 7u, 34u)));
  } else {
    v_2 = false;
  }
  bool has_resource = v_2;
  uint v_3 = 0u;
  if (has_resource) {
    v_3 = tint_resource_table_metadata.Load(4u);
  } else {
    v_3 = 6u;
  }
  uint texture_kind = v_3;
  uint v_4 = 0u;
  if (has_resource) {
    v_4 = v;
  } else {
    v_4 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_4;
  bool v_5 = false;
  if ((v_1 < tint_resource_table_metadata.Load(0u))) {
    v_5 = any((uint2((tint_resource_table_metadata.Load(8u)).xx) == uint2(40u, 41u)));
  } else {
    v_5 = false;
  }
  bool has_resource_1 = v_5;
  uint v_6 = 0u;
  if (has_resource_1) {
    v_6 = tint_resource_table_metadata.Load(8u);
  } else {
    v_6 = 41u;
  }
  uint sampler_kind = v_6;
  uint v_7 = 0u;
  if (has_resource_1) {
    v_7 = v_1;
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

