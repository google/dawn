enable f16;

fn cross_9857cb() {
  var res : vec3<f16> = cross(vec3<f16>(1.0h), vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

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
