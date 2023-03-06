@group(1) @binding(1) var arg_1 : texture_cube<i32>;

@group(1) @binding(2) var arg_2 : sampler;

fn textureGather_5ba85f() {
  const arg_0 = 1i;
  var arg_3 = vec3<f32>(1.0f);
  var res : vec4<i32> = textureGather(arg_0, arg_1, arg_2, arg_3);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureGather_5ba85f();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureGather_5ba85f();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureGather_5ba85f();
}
