enable f16;

fn cross_9857cb() {
  var arg_0 = vec3<f16>();
  var arg_1 = vec3<f16>();
  var res : vec3<f16> = cross(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  cross_9857cb();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  cross_9857cb();
}

@compute @workgroup_size(1)
fn compute_main() {
  cross_9857cb();
}
