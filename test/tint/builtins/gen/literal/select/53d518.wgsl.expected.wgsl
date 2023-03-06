enable f16;

fn select_53d518() {
  var res : vec3<f16> = select(vec3<f16>(1.0h), vec3<f16>(1.0h), vec3<bool>(true));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

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
