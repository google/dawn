fn length_555aba() {
  const arg_0 = vec3(0.0);
  var res = length(arg_0);
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  length_555aba();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  length_555aba();
}

@compute @workgroup_size(1)
fn compute_main() {
  length_555aba();
}
