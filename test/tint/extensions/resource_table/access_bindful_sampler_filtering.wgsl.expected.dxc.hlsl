struct fs_outputs {
  float4 tint_symbol : SV_Target0;
};


SamplerState s : register(s0);
Texture2D<float4> tint_resource_table_array[] : register(t28, space4);
Texture2D tint_resource_table_array_1[] : register(t28, space6);
SamplerState tint_resource_table_array_2[] : register(s28, space7);
ByteAddressBuffer tint_resource_table_metadata : register(t29, space5);
float4 fs_inner() {
  uint v = uint(int(0));
  bool v_1 = false;
  if ((v < tint_resource_table_metadata.Load(0u))) {
    v_1 = any((uint3((tint_resource_table_metadata.Load(4u)).xxx) == uint3(6u, 7u, 34u)));
  } else {
    v_1 = false;
  }
  bool has_resource = v_1;
  uint v_2 = 0u;
  if (has_resource) {
    v_2 = tint_resource_table_metadata.Load(4u);
  } else {
    v_2 = 6u;
  }
  uint texture_kind = v_2;
  uint v_3 = 0u;
  if (has_resource) {
    v_3 = v;
  } else {
    v_3 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint item_idx = v_3;
  float4 v_4 = (0.0f).xxxx;
  if ((texture_kind == 6u)) {
    v_4 = tint_resource_table_array[item_idx].Sample(s, (0.0f).xx);
  } else {
    v_4 = (0.0f).xxxx;
  }
  return v_4;
}

fs_outputs fs() {
  fs_outputs v_5 = {fs_inner()};
  return v_5;
}

