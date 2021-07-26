intrinsics/gen/textureLoad/8e5032.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<u32> = textureLoad(arg_0, vec2<i32>(), 1);
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_2d_array<rg32uint, read>;

fn textureLoad_8e5032() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_8e5032();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_8e5032();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_8e5032();
}
