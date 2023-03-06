fn smoothstep_40864c() {
  var arg_0 = vec4<f32>(2.0f);
  var arg_1 = vec4<f32>(4.0f);
  var arg_2 = vec4<f32>(3.0f);
  var res : vec4<f32> = smoothstep(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothstep_40864c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothstep_40864c();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_40864c();
}
