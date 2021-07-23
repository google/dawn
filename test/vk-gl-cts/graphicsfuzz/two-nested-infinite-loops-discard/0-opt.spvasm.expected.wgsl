[[block]]
struct buf0 {
  injectionSwitch : vec2<f32>;
};

[[group(0), binding(0)]] var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn mand_() -> vec3<f32> {
  var k : i32;
  k = 0;
  loop {
    if (true) {
    } else {
      break;
    }
    discard;
  }
  return vec3<f32>(1.0, 1.0, 1.0);
}

fn main_1() {
  var j : i32;
  let x_37 : f32 = x_7.injectionSwitch.x;
  let x_39 : f32 = x_7.injectionSwitch.y;
  if ((x_37 > x_39)) {
    j = 0;
    loop {
      if (true) {
      } else {
        break;
      }

      continuing {
        let x_46 : vec3<f32> = mand_();
      }
    }
  }
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
