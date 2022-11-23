fn frexp_bee870() {
  var res = frexp(1.0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  frexp_bee870();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  frexp_bee870();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_bee870();
}
