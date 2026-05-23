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
  bool v_4 = v_2;
  uint v_5 = 0u;
  if (v_4) {
    v_5 = tint_resource_table_metadata.Load((4u + (v * 4u)));
  } else {
    v_5 = 6u;
  }
  uint texture_kind = v_5;
  uint v_6 = 0u;
  if (v_4) {
    v_6 = v;
  } else {
    v_6 = (0u + tint_resource_table_metadata.Load(0u));
  }
  uint v_7 = v_6;
  bool v_8 = false;
  if ((v_1 < tint_resource_table_metadata.Load(0u))) {
    uint2 v_9 = uint2((tint_resource_table_metadata.Load((4u + (v_1 * 4u)))).xx);
    v_8 = any((v_9 == uint2(40u, 41u)));
  } else {
    v_8 = false;
  }
  bool v_10 = v_8;
  uint v_11 = 0u;
  if (v_10) {
    v_11 = tint_resource_table_metadata.Load((4u + (v_1 * 4u)));
  } else {
    v_11 = 41u;
  }
  uint sampler_kind = v_11;
  uint v_12 = 0u;
  if (v_10) {
    v_12 = v_1;
  } else {
    v_12 = (4u + tint_resource_table_metadata.Load(0u));
  }
  uint v_13 = v_12;
  bool v_14 = false;
  if ((sampler_kind == 40u)) {
    v_14 = (texture_kind == 6u);
  } else {
    v_14 = true;
  }
  float4 v_15 = (0.0f).xxxx;
  if (v_14) {
    v_15 = tint_resource_table_array[v_7].Sample(tint_resource_table_array_2[v_13], (0.0f).xx);
  } else {
    uint v_16 = (4u + tint_resource_table_metadata.Load(0u));
    v_15 = tint_resource_table_array[v_7].Sample(tint_resource_table_array_2[v_16], (0.0f).xx);
  }
  return v_15;
}

fs_outputs fs() {
  fs_outputs v_17 = {fs_inner()};
  return v_17;
}

