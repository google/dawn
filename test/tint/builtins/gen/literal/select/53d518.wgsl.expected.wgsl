enable f16;

fn select_53d518() {
  var res : vec3<f16> = select(vec3<f16>(f16()), vec3<f16>(f16()), vec3<bool>(true));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  select_53d518();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  select_53d518();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_53d518();
}
