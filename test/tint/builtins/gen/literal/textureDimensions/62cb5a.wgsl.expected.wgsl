@group(1) @binding(0) var arg_0 : texture_2d_array<i32>;

fn textureDimensions_62cb5a() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_62cb5a();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_62cb5a();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_62cb5a();
}
