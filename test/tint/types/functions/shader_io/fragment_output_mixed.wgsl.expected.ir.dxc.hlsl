struct FragmentOutputs {
  int loc0;
  float frag_depth;
  uint loc1;
  float loc2;
  uint sample_mask;
  float4 loc3;
};

struct main_outputs {
  int FragmentOutputs_loc0 : SV_Target0;
  uint FragmentOutputs_loc1 : SV_Target1;
  float FragmentOutputs_loc2 : SV_Target2;
  float4 FragmentOutputs_loc3 : SV_Target3;
  float FragmentOutputs_frag_depth : SV_Depth;
  uint FragmentOutputs_sample_mask : SV_Coverage;
};


FragmentOutputs main_inner() {
  FragmentOutputs v = {int(1), 2.0f, 1u, 1.0f, 2u, float4(1.0f, 2.0f, 3.0f, 4.0f)};
  return v;
}

main_outputs main() {
  FragmentOutputs v_1 = main_inner();
  FragmentOutputs v_2 = v_1;
  FragmentOutputs v_3 = v_1;
  FragmentOutputs v_4 = v_1;
  FragmentOutputs v_5 = v_1;
  FragmentOutputs v_6 = v_1;
  FragmentOutputs v_7 = v_1;
  main_outputs v_8 = {v_2.loc0, v_4.loc1, v_5.loc2, v_7.loc3, v_3.frag_depth, v_6.sample_mask};
  return v_8;
}

