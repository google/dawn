[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_2d<rgba8uint>;

fn textureLoad_3c0d9e() {
  var res : vec4<u32> = textureLoad(arg_0, vec2<i32>());
}

[[stage(vertex)]]
fn vertex_main() {
  textureLoad_3c0d9e();
}

[[stage(fragment)]]
fn fragment_main() {
  textureLoad_3c0d9e();
}

[[stage(compute)]]
fn compute_main() {
  textureLoad_3c0d9e();
}
