intrinsics/gen/textureLoad/e65916.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<i32> = textureLoad(arg_0, vec3<i32>());
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rg32sint, read>;

fn textureLoad_e65916() {
  var res : vec4<i32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_e65916();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_e65916();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_e65916();
}
