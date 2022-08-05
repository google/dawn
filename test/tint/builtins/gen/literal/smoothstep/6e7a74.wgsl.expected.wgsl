enable f16;

fn smoothstep_6e7a74() {
  var res : vec3<f16> = smoothstep(vec3<f16>(f16()), vec3<f16>(f16()), vec3<f16>(f16()));
}

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
