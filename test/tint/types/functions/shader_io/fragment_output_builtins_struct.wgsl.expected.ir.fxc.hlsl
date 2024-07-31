struct FragmentOutputs {
  float frag_depth;
  uint sample_mask;
};

struct main_outputs {
  float FragmentOutputs_frag_depth : SV_Depth;
  uint FragmentOutputs_sample_mask : SV_Coverage;
};


FragmentOutputs main_inner() {
  FragmentOutputs v = {1.0f, 1u};
  return v;
}

main_outputs main() {
  FragmentOutputs v_1 = main_inner();
  FragmentOutputs v_2 = v_1;
  FragmentOutputs v_3 = v_1;
  main_outputs v_4 = {v_2.frag_depth, v_3.sample_mask};
  return v_4;
}

