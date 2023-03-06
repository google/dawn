enable f16;

fn smoothstep_6e7a74() {
  var res : vec3<f16> = smoothstep(vec3<f16>(2.0h), vec3<f16>(4.0h), vec3<f16>(3.0h));
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f16>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_6e7a74();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_6e7a74();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_6e7a74();
}
