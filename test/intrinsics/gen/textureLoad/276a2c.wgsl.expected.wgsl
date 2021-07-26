intrinsics/gen/textureLoad/276a2c.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<u32> = textureLoad(arg_0, 1);
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_1d<rgba32uint, read>;

fn textureLoad_276a2c() {
  var res : vec4<u32> = textureLoad(arg_0, 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_276a2c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_276a2c();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_276a2c();
}
