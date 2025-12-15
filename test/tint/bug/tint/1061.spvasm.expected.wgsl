struct buf0 {
  r : vec4<f32>,
}

@group(0u) @binding(0u) var<uniform> v_1 : buf0;

var<private> _GLF_color : vec4<f32>;

fn main_inner() {
  var f : f32;
  var v : vec4<f32>;
  f = determinant(mat3x3<f32>(vec3<f32>(1.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 1.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 1.0f)));
  v = vec4<f32>(sin(f), cos(f), exp2(f), log(f));
  if ((distance(v, v_1.r) < 0.10000000149011611938f)) {
    _GLF_color = vec4<f32>(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    _GLF_color = vec4<f32>();
  }
}

@fragment
fn main() -> @location(0u) vec4<f32> {
  main_inner();
  return _GLF_color;
}
