[[block]]
struct buf0 {
  one : i32;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  a = 0;
  let x_26 : i32 = x_6.one;
  switch(x_26) {
    case 2, 3: {
      a = 1;
      fallthrough;
    }
    case 4: {
    }
    default: {
      a = 2;
    }
  }
  let x_31 : i32 = a;
  if ((x_31 == 2)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
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
