@group(1) @binding(0) var arg_0 : texture_2d<u32>;

fn textureDimensions_e18a8b() {
  var arg_1 = 1u;
  var res : vec2<u32> = textureDimensions(arg_0, arg_1);
  prevent_dce = res;
}

@group(2) @binding(0) var<storage, read_write> prevent_dce : vec2<u32>;

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  textureDimensions_e18a8b();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  textureDimensions_e18a8b();
}

@compute @workgroup_size(1)
fn compute_main() {
  textureDimensions_e18a8b();
}
