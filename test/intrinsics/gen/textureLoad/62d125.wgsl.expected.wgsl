intrinsics/gen/textureLoad/62d125.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<f32> = textureLoad(arg_0, vec3<i32>());
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_3d<rgba8snorm, read>;

fn textureLoad_62d125() {
  var res : vec4<f32> = textureLoad(arg_0, vec3<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_62d125();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_62d125();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_62d125();
}
