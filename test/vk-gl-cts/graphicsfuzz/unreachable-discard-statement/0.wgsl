var<private> x_GLF_color : vec4<f32>;

fn mand_() -> vec3<f32> {
  var x_40 : bool = false;
  var x_41 : vec3<f32>;
  var k : i32;
  loop {
    k = 0;
    loop {
      let x_7 : i32 = k;
      if ((x_7 < 1000)) {
      } else {
        break;
      }
      x_40 = true;
      x_41 = vec3<f32>(1.0, 1.0, 1.0);
      break;
    }
    let x_50 : bool = x_40;
    if (x_50) {
      break;
    }
    discard;
  }
  let x_52 : vec3<f32> = x_41;
  return x_52;
}

fn main_1() {
  var i : i32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  i = 0;
  loop {
    let x_8 : i32 = i;
    if ((x_8 < 4)) {
    } else {
      break;
    }

    continuing {
      let x_38 : vec3<f32> = mand_();
      let x_9 : i32 = i;
      i = (x_9 + 1);
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
