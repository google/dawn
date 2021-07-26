intrinsics/gen/textureLoad/c02b74.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<f32> = textureLoad(arg_0, 1);
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_1d<rgba16float, read>;

fn textureLoad_c02b74() {
  var res : vec4<f32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_c02b74();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_c02b74();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_c02b74();
}
