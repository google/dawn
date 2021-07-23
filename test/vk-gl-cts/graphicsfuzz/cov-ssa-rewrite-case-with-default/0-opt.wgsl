[[block]]
struct buf0 {
  one : i32;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> vec4<f32> {
  let x_48 : i32 = x_6.one;
  if ((x_48 == 1)) {
    return vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    return vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
  return vec4<f32>(0.0, 0.0, 0.0, 0.0);
}

fn main_1() {
  var i : i32;
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  i = 0;
  loop {
    let x_33 : i32 = i;
    let x_35 : i32 = x_6.one;
    if ((x_33 <= x_35)) {
    } else {
      break;
    }
    let x_38 : i32 = i;
    switch(x_38) {
      case 1: {
        let x_43 : vec4<f32> = func_();
        x_GLF_color = x_43;
        fallthrough;
      }
      default: {
        fallthrough;
      }
      case 0: {
        x_GLF_color.y = 0.0;
      }
    }

    continuing {
      let x_44 : i32 = i;
      i = (x_44 + 1);
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
