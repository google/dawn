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
  bool v_2 = v;
  uint v_3 = 0u;
  if (v_2) {
    v_3 = tint_resource_table_metadata.Load(12u);
  } else {
    v_3 = 6u;
  }
  uint texture_kind = v_3;
  uint v_4 = 0u;
  if (v_2) {
    v_4 = 2u;
  } else {
    v_4 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint v_5 = v_4;
  bool v_6 = false;
  if ((3u < tint_resource_table_metadata.Load(0u))) {
    uint2 v_7 = uint2((tint_resource_table_metadata.Load(16u)).xx);
    v_6 = any((v_7 == uint2(40u, 41u)));
  } else {
    v_6 = false;
  }
  bool v_8 = v_6;
  uint v_9 = 0u;
  if (v_8) {
    v_9 = tint_resource_table_metadata.Load(16u);
  } else {
    v_9 = 41u;
  }
  uint sampler_kind = v_9;
  uint v_10 = 0u;
  if (v_8) {
    v_10 = 3u;
  } else {
    v_10 = (4u + tint_resource_table_metadata.Load(0u));
  }
  uint v_11 = v_10;
  bool v_12 = false;
  if ((sampler_kind == 40u)) {
    v_12 = (texture_kind == 6u);
  } else {
    v_12 = true;
  }
  float4 v_13 = (0.0f).xxxx;
  if (v_12) {
    v_13 = tint_resource_table_array[v_5].Sample(tint_resource_table_array_2[v_11], (0.0f).xx);
  } else {
    uint v_14 = (4u + tint_resource_table_metadata.Load(0u));
    v_13 = tint_resource_table_array[v_5].Sample(tint_resource_table_array_2[v_14], (0.0f).xx);
  }
  return v_13;
}

fs_outputs fs() {
  fs_outputs v_15 = {fs_inner()};
  return v_15;
}

