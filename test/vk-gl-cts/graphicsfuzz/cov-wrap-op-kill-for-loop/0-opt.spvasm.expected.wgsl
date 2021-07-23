[[block]]
struct buf0 {
  zero : i32;
};

[[group(0), binding(0)]] var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_i1_(x : ptr<function, i32>) {
  let x_41 : i32 = *(x);
  let x_43 : i32 = x_7.zero;
  if ((x_41 < x_43)) {
    discard;
  }
  let x_47 : i32 = *(x);
  if ((x_47 > 8)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
  return;
}

fn main_1() {
  var i : i32;
  var param : i32;
  var x_31_phi : i32;
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  i = 0;
  x_31_phi = 0;
  loop {
    let x_31 : i32 = x_31_phi;
    let x_35 : i32 = x_7.zero;
    if ((x_31 < (10 + x_35))) {
    } else {
      break;
    }

    continuing {
      param = x_31;
      func_i1_(&(param));
      let x_32 : i32 = (x_31 + 1);
      i = x_32;
      x_31_phi = x_32;
    }
  }
  return;
}

struct main_out {
  [[location(0)]]
  x_GLF_color_1 : vec4<f32>;
};

[[stage(fragment)]]
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
