//
// less
//
struct FragDepthLessOutput {
  float frag_depth;
};

struct less_outputs {
  float FragDepthLessOutput_frag_depth : SV_DepthLessEqual;
};


FragDepthLessOutput less_inner() {
  FragDepthLessOutput v = {1.0f};
  return v;
}

less_outputs less() {
  FragDepthLessOutput v_1 = less_inner();
  less_outputs v_2 = {v_1.frag_depth};
  return v_2;
}

//
// greater
//
struct FragDepthGreaterOutput {
  float frag_depth;
};

struct greater_outputs {
  float FragDepthGreaterOutput_frag_depth : SV_DepthGreaterEqual;
};


FragDepthGreaterOutput greater_inner() {
  FragDepthGreaterOutput v = {1.0f};
  return v;
}

greater_outputs greater() {
  FragDepthGreaterOutput v_1 = greater_inner();
  greater_outputs v_2 = {v_1.frag_depth};
  return v_2;
}

