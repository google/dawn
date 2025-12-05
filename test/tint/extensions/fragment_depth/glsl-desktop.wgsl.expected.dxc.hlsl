//
// any
//
struct any_outputs {
  float tint_symbol : SV_Depth;
};


float any_inner() {
  return 1.0f;
}

any_outputs any() {
  any_outputs v = {any_inner()};
  return v;
}

//
// less
//
struct less_outputs {
  float tint_symbol : SV_DepthLessEqual;
};


float less_inner() {
  return 1.0f;
}

less_outputs less() {
  less_outputs v = {less_inner()};
  return v;
}

//
// greater
//
struct greater_outputs {
  float tint_symbol : SV_DepthGreaterEqual;
};


float greater_inner() {
  return 1.0f;
}

greater_outputs greater() {
  greater_outputs v = {greater_inner()};
  return v;
}

