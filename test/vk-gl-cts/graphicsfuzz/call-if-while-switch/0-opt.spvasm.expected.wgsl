[[block]]
struct buf0 {
  injectionSwitch : vec2<f32>;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var data : array<i32, 10>;
  var x_40 : i32;
  var x_40_phi : i32;
  var x_11_phi : i32;
  let x_7 : i32 = data[1];
  let x_10 : i32 = select(1, 2, (1 < x_7));
  x_40_phi = 1;
  x_11_phi = x_10;
  loop {
    var x_54 : i32;
    var x_41 : i32;
    var x_41_phi : i32;
    x_40 = x_40_phi;
    let x_11 : i32 = x_11_phi;
    if ((x_11 < 3)) {
    } else {
      break;
    }
    var x_54_phi : i32;
    let x_8 : i32 = (x_11 + 1);
    let x_47 : f32 = x_6.injectionSwitch.x;
    x_54_phi = x_40;
    switch(i32(x_47)) {
      case 78: {
        x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
        fallthrough;
      }
      case 19: {
        x_54_phi = bitcast<i32>((x_40 + bitcast<i32>(1)));
        fallthrough;
      }
      case 23, 38: {
        x_54 = x_54_phi;
        x_41_phi = x_54;
        continue;
      }
      default: {
        x_41_phi = x_40;
        continue;
      }
    }
    x_41_phi = 0;

    continuing {
      x_41 = x_41_phi;
      x_40_phi = x_41;
      x_11_phi = x_8;
    }
  }
  data[x_40] = 1;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
