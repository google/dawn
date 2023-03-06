fn mix_0c8c33() {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = vec3<f32>(1.0f);
  var arg_2 = vec3<f32>(1.0f);
  var res : vec3<f32> = mix(arg_0, arg_1, arg_2);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  mix_0c8c33();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  mix_0c8c33();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_0c8c33();
}
