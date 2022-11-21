fn radians_524a91() {
  var res = radians(vec4(1.0));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  radians_524a91();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  radians_524a91();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_524a91();
}
