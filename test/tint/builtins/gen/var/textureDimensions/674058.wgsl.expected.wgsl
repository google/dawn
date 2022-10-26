@group(1) @binding(0) var arg_0 : texture_storage_2d_array<r32sint, write>;

fn textureDimensions_674058() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_674058();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_674058();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_674058();
}
