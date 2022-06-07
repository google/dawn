@group(1) @binding(0) var arg_0 : texture_storage_2d_array<rgba32float, write>;

fn textureDimensions_9abfe5() {
  var res : vec2<i32> = textureDimensions(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_9abfe5();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_9abfe5();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_9abfe5();
}
