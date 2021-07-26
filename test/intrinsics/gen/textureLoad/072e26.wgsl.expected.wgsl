intrinsics/gen/textureLoad/072e26.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<f32> = textureLoad(arg_0, vec2<i32>(), 1);
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_2d_array<rgba8snorm, read>;

fn textureLoad_072e26() {
  var res : vec4<f32> = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_072e26();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_072e26();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_072e26();
}
