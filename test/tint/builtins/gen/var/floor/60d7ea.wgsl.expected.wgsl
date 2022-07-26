fn floor_60d7ea() {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = floor(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  floor_60d7ea();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  floor_60d7ea();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_60d7ea();
}
