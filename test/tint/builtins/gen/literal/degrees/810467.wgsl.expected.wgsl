fn degrees_810467() {
  var res = degrees(vec2(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  degrees_810467();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  degrees_810467();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_810467();
}
