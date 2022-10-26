@group(1) @binding(0) var arg_0 : texture_storage_1d<r32uint, write>;

fn textureDimensions_7228de() {
  var res : u32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_7228de();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_7228de();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_7228de();
}
