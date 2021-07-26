intrinsics/gen/textureLoad/c9cc40.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<i32> = textureLoad(arg_0, 1);
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_1d<rgba8sint, read>;

fn textureLoad_c9cc40() {
  var res : vec4<i32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_c9cc40();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_c9cc40();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_c9cc40();
}
