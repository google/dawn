type Arr = [[stride(16)]] array<i32, 5>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr;
};

[[group(0), binding(0)]] var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var count0 : i32;
  var count1 : i32;
  var i : i32;
  let x_29 : i32 = x_6.x_GLF_uniform_int_values[2];
  count0 = x_29;
  let x_31 : i32 = x_6.x_GLF_uniform_int_values[2];
  count1 = x_31;
  let x_33 : i32 = x_6.x_GLF_uniform_int_values[2];
  i = x_33;
  loop {
    let x_38 : i32 = i;
    let x_40 : i32 = x_6.x_GLF_uniform_int_values[4];
    if ((x_38 < x_40)) {
    } else {
      break;
    }
    let x_43 : i32 = i;
    switch(x_43) {
      case 0, 1: {
        let x_47 : i32 = count0;
        count0 = (x_47 + 1);
        fallthrough;
      }
      case 2, 3: {
        let x_49 : i32 = count1;
        count1 = (x_49 + 1);
      }
      default: {
      }
    }

    continuing {
      let x_51 : i32 = i;
      i = (x_51 + 1);
    }
  }
  let x_53 : i32 = count1;
  let x_55 : i32 = x_6.x_GLF_uniform_int_values[0];
  if ((x_53 == x_55)) {
    let x_61 : i32 = x_6.x_GLF_uniform_int_values[3];
    let x_64 : i32 = x_6.x_GLF_uniform_int_values[2];
    let x_67 : i32 = x_6.x_GLF_uniform_int_values[2];
    let x_70 : i32 = x_6.x_GLF_uniform_int_values[3];
    x_GLF_color = vec4<f32>(f32(x_61), f32(x_64), f32(x_67), f32(x_70));
  } else {
    let x_74 : i32 = x_6.x_GLF_uniform_int_values[2];
    let x_75 : f32 = f32(x_74);
    x_GLF_color = vec4<f32>(x_75, x_75, x_75, x_75);
  }
  let x_77 : i32 = count0;
  let x_79 : i32 = x_6.x_GLF_uniform_int_values[1];
  if ((x_77 != x_79)) {
    let x_84 : i32 = x_6.x_GLF_uniform_int_values[2];
    let x_85 : f32 = f32(x_84);
    x_GLF_color = vec4<f32>(x_85, x_85, x_85, x_85);
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
