[[group(1), binding(0)]] var arg_0 : texture_storage_2d_array<rg32float, write>;

fn textureDimensions_fb5670() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

[[stage(vertex)]]
fn vertex_main() -> [[builtin(position)]] vec4<f32> {
  textureDimensions_fb5670();
  return vec4<f32>();
}

[[stage(fragment)]]
fn fragment_main() {
  textureDimensions_fb5670();
}

[[stage(compute), workgroup_size(1)]]
fn compute_main() {
  textureDimensions_fb5670();
}
