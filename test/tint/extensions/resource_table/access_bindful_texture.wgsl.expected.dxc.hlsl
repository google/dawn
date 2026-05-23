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
  bool v_3 = v_1;
  uint v_4 = 0u;
  if (v_3) {
    v_4 = tint_resource_table_metadata.Load((4u + (v * 4u)));
  } else {
    v_4 = 41u;
  }
  uint sampler_kind = v_4;
  uint v_5 = 0u;
  if (v_3) {
    v_5 = v;
  } else {
    v_5 = (4u + tint_resource_table_metadata.Load(0u));
  }
  uint v_6 = v_5;
  bool v_7 = false;
  if ((sampler_kind == 40u)) {
    v_7 = true;
  } else {
    v_7 = true;
  }
  float4 v_8 = (0.0f).xxxx;
  if (v_7) {
    v_8 = t.Sample(tint_resource_table_array_2[v_6], (0.0f).xx);
  } else {
    uint v_9 = (4u + tint_resource_table_metadata.Load(0u));
    v_8 = t.Sample(tint_resource_table_array_2[v_9], (0.0f).xx);
  }
  return v_8;
}

fs_outputs fs() {
  fs_outputs v_10 = {fs_inner()};
  return v_10;
}

