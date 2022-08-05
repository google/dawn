enable f16;

fn radians_7ea4c7() {
  var arg_0 = vec3<f16>(f16());
  var res : vec3<f16> = radians(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_7ea4c7();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_7ea4c7();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_7ea4c7();
}
