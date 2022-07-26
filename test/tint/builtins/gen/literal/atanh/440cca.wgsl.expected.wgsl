fn atanh_440cca() {
  var res : vec3<f32> = atanh(vec3<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  atanh_440cca();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  atanh_440cca();
}

@compute @workgroup_size(1)
fn compute_main() {
  atanh_440cca();
}
