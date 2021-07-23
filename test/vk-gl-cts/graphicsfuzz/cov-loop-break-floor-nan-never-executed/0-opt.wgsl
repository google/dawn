[[block]]
struct buf0 {
  one : i32;
};

[[group(0), binding(0)]] var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var i : i32;
  a = 0;
  i = 0;
  loop {
    let x_32 : i32 = i;
    if ((x_32 < 5)) {
    } else {
      break;
    }
    let x_36 : i32 = x_7.one;
    if ((x_36 == 0)) {
      if ((floor(bitcast<f32>(-4194304)) > 0.0)) {
        a = -1;
        break;
      }
    }
    let x_45 : i32 = a;
    a = (x_45 + 1);

    continuing {
      let x_47 : i32 = i;
      i = (x_47 + 1);
    }
  }
  let x_49 : i32 = a;
  if ((x_49 == 5)) {
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
