struct buf0 {
  /* @offset(0) */
  r : vec4f,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4f;

fn main_1() {
  var f : f32;
  var v : vec4f;
  f = determinant(mat3x3f(vec3f(1.0f, 0.0f, 0.0f), vec3f(0.0f, 1.0f, 0.0f), vec3f(0.0f, 0.0f, 1.0f)));
  let x_33 = f;
  let x_35 = f;
  let x_37 = f;
  let x_39 = f;
  v = vec4f(sin(x_33), cos(x_35), exp2(x_37), log(x_39));
  let x_42 = v;
  let x_44 = x_7.r;
  if ((distance(x_42, x_44) < 0.10000000149011611938f)) {
    x_GLF_color = vec4f(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = vec4f();
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4f,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
