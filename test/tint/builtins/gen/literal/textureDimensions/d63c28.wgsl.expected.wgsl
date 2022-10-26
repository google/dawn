@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32float, write>;

fn textureDimensions_d63c28() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_d63c28();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_d63c28();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_d63c28();
}
