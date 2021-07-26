intrinsics/gen/textureLoad/fdebd0.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<u32> = textureLoad(arg_0, vec2<i32>(), 1);
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_2d_array<rgba8uint, read>;

fn textureLoad_fdebd0() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_fdebd0();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_fdebd0();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_fdebd0();
}
