@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32sint, write>;

fn textureDimensions_c44fc1() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_c44fc1();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_c44fc1();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_c44fc1();
}
