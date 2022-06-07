@group(1) @binding(0) var arg_0 : texture_storage_2d<rgba8sint, write>;

fn textureDimensions_3c5ad8() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_3c5ad8();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_3c5ad8();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_3c5ad8();
}
