intrinsics/gen/textureLoad/3c9587.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<f32> = textureLoad(arg_0, vec2<i32>());
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_2d<rgba8unorm, read>;

fn textureLoad_3c9587() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_3c9587();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_3c9587();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_3c9587();
}
