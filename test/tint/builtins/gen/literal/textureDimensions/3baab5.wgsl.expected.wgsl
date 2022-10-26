@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba8uint, write>;

fn textureDimensions_3baab5() {
  var res : vec2<u32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_3baab5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_3baab5();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_3baab5();
}
