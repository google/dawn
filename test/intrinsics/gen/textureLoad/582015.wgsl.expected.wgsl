intrinsics/gen/textureLoad/582015.wgsl:29:24 warning: use of deprecated intrinsic
  var res: vec4<i32> = textureLoad(arg_0, vec2<i32>(), 1);
                       ^^^^^^^^^^^

[[group(1), binding(0)]] var arg_0 : texture_storage_2d_array<rgba8sint, read>;

fn textureLoad_582015() {
  var res : vec4<i32> = textureLoad(arg_0, vec2<i32>(), 1);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureLoad_582015();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_582015();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureLoad_582015();
}
