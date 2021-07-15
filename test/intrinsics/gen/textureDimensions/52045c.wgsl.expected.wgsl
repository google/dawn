[[group(1), binding(0)]] var arg_0 : texture_1d<i32>;

fn textureDimensions_52045c() {
  var res : i32 = textureDimensions(arg_0, 0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_52045c();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_52045c();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_52045c();
}
