fn max_de6b87() {
  var res = max(vec2(1.0), vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  max_de6b87();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  max_de6b87();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_de6b87();
}
