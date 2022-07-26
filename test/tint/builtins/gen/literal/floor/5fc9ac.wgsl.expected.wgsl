fn floor_5fc9ac() {
  var res : vec2<f32> = floor(vec2<f32>(1.0f));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_5fc9ac();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_5fc9ac();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_5fc9ac();
}
