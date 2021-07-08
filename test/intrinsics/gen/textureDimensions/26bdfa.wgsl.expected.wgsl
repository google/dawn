[[group(1), binding(0)]] var arg_0 : texture_3d<f32>;

fn textureDimensions_26bdfa() {
  var res : vec3<i32> = textureDimensions(arg_0, 0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_26bdfa();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_26bdfa();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_26bdfa();
}
