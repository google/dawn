fn sin_68d3ab() {
  var res = sin(vec2(1.57079632679000003037));
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  sin_68d3ab();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  sin_68d3ab();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_68d3ab();
}
