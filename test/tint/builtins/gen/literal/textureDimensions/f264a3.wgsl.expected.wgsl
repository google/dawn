@group(1) @binding(0) var arg_0 : texture_storage_1d<rg32sint, write>;

fn textureDimensions_f264a3() {
  var res : u32 = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_f264a3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_f264a3();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_f264a3();
}
