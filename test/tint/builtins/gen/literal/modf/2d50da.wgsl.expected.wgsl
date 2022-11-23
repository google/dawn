fn modf_2d50da() {
  var res = modf(vec2<f32>(-(1.5f)));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  modf_2d50da();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  modf_2d50da();
}

@compute @workgroup_size(1)
fn compute_main() {
  modf_2d50da();
}
