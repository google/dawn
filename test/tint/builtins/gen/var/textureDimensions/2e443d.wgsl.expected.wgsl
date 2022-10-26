@group(1) @binding(0) var arg_0 : texture_2d<i32>;

fn textureDimensions_2e443d() {
  var arg_1 = 1i;
  var res : vec2<u32> = textureDimensions(arg_0, arg_1);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_2e443d();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_2e443d();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_2e443d();
}
