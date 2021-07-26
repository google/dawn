intrinsics/gen/textureLoad/eb573b.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<i32> = textureLoad(arg_0, vec2<i32>());
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_2d<r32sint, read>;

fn textureLoad_eb573b() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_eb573b();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_eb573b();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_eb573b();
}
