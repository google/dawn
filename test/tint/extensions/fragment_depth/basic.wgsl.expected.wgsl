requires fragment_depth;

@fragment
fn any() -> @builtin(frag_depth) f32 {
  return 1.0;
}

@fragment
fn less() -> @builtin(frag_depth) f32 {
  return 1.0;
}

@fragment
fn greater() -> @builtin(frag_depth) f32 {
  return 1.0;
}
