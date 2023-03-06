enable f16;

fn saturate_462535() {
  var res : vec3<f16> = saturate(vec3<f16>(2.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  saturate_462535();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  saturate_462535();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_462535();
}
