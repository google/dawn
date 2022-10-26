@group(1) @binding(0) var arg_0 : texture_storage_1d<rgba16float, write>;

fn textureDimensions_0c0b0c() {
  var res : u32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_0c0b0c();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_0c0b0c();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_0c0b0c();
}
