builtins/gen/literal/smoothstep/658be3.wgsl:28:24 warning: use of deprecated builtin
  var res: vec3<f32> = smoothStep(vec3<f32>(), vec3<f32>(), vec3<f32>());
                       ^^^^^^^^^^

fn smoothStep_658be3() {
  var res : vec3<f32> = smoothStep(vec3<f32>(), vec3<f32>(), vec3<f32>());
}

@vertex
fn vertex_main() -> @builtin(position) vec4<f32> {
  smoothStep_658be3();
  return vec4<f32>();
}

@fragment
fn fragment_main() {
  smoothStep_658be3();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothStep_658be3();
}
