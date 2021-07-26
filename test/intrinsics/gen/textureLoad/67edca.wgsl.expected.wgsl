intrinsics/gen/textureLoad/67edca.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<u32> = textureLoad(arg_0, vec3<i32>());
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rgba32uint, read>;

fn textureLoad_67edca() {
  var res : vec4<u32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_67edca();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_67edca();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_67edca();
}
