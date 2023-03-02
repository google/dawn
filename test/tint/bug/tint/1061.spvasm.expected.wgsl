struct buf0 {
  /* @offset(0) */
  r : vec4<f32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var v : vec4<f32>;
  f = determinant(mat3x3<f32>(vec3<f32>(1.0f, 0.0f, 0.0f), vec3<f32>(0.0f, 1.0f, 0.0f), vec3<f32>(0.0f, 0.0f, 1.0f)));
  let x_33 : f32 = f;
  let x_35 : f32 = f;
  let x_37 : f32 = f;
  let x_39 : f32 = f;
  v = vec4<f32>(sin(x_33), cos(x_35), exp2(x_37), log(x_39));
  let x_42 : vec4<f32> = v;
  let x_44 : vec4<f32> = x_7.r;
  if ((distance(x_42, x_44) < 0.10000000149011611938f)) {
    x_GLF_color = vec4<f32>(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4<f32>(0.0f, 0.0f, 0.0f, 0.0f);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
