enable f16;

fn normalize_39d5ec() {
  var arg_0 = vec3<f16>(1.0h);
  var res : vec3<f16> = normalize(arg_0);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  normalize_39d5ec();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  normalize_39d5ec();
}

@compute @workgroup_size(1)
fn compute_main() {
  normalize_39d5ec();
}
