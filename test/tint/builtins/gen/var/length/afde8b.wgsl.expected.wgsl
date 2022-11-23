fn length_afde8b() {
  var arg_0 = vec2<f32>(0.0f);
  var res : f32 = length(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_afde8b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_afde8b();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_afde8b();
}
