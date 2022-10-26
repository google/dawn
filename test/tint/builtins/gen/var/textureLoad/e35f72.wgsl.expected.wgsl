@group(1) @binding(0) var arg_0 : texture_3d<i32>;

fn textureLoad_e35f72() {
  var arg_1 = vec3<i32>();
  var arg_2 = 1u;
  var res : vec4<i32> = textureLoad(arg_0, arg_1, arg_2);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureLoad_e35f72();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureLoad_e35f72();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureLoad_e35f72();
}
