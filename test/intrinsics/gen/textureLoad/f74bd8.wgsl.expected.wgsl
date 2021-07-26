intrinsics/gen/textureLoad/f74bd8.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<f32> = textureLoad(arg_0, vec3<i32>());
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rg32float, read>;

fn textureLoad_f74bd8() {
  var res : vec4<f32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_f74bd8();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_f74bd8();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_f74bd8();
}
