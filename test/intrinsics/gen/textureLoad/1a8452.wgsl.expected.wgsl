intrinsics/gen/textureLoad/1a8452.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<u32> = textureLoad(arg_0, 1);
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_1d<rgba8uint, read>;

fn textureLoad_1a8452() {
  var res : vec4<u32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_1a8452();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_1a8452();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_1a8452();
}
