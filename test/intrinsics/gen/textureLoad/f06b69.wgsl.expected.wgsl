intrinsics/gen/textureLoad/f06b69.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<i32> = textureLoad(arg_0, 1);
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_1d<r32sint, read>;

fn textureLoad_f06b69() {
  var res : vec4<i32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_f06b69();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_f06b69();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_f06b69();
}
