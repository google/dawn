@group(1) @binding(0) var arg_0 : texture_2d_array<f32>;

fn textureLoad_3c96e8() {
  var arg_1 = vec2<i32>(1i);
  var arg_2 = 1u;
  var arg_3 = 1u;
  var res : vec4<f32> = textureLoad(arg_0, arg_1, arg_2, arg_3);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_3c96e8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_3c96e8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_3c96e8();
}
