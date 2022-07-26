fn floor_5fc9ac() {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = floor(arg_0);
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
