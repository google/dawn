[[group(1), binding(0)]] var arg_0 : [[access(read)]] texture_storage_3d<r32sint>;

fn textureDimensions_1e189c() {
  var res : vec3<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_1e189c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_1e189c();
}

[[stage(compute)]]
fn compute_main() {
  textureDimensions_1e189c();
}
