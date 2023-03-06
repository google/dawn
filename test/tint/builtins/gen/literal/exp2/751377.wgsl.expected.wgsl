enable f16;

fn exp2_751377() {
  var res : vec3<f16> = exp2(vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  exp2_751377();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  exp2_751377();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_751377();
}
