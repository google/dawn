enable f16;

fn sqrt_895a0c() {
  var res : vec3<f16> = sqrt(vec3<f16>(1.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sqrt_895a0c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sqrt_895a0c();
}

@compute @workgroup_size(1)
fn compute_main() {
  sqrt_895a0c();
}
