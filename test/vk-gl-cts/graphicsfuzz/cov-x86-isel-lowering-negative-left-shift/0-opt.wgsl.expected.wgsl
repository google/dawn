type Arr = [[stride(16)]] array<f32, 3>;

[[block]]
struct buf1 {
  x_GLF_uniform_float_values : Arr;
};

type Arr_1 = [[stride(16)]] array<i32, 4>;

[[block]]
struct buf0 {
  x_GLF_uniform_int_values : Arr_1;
};

[[group(0), binding(1)]] var<uniform> x_6 : buf1;

[[group(0), binding(0)]] var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var A : array<f32, 2>;
  var i : i32;
  var j : i32;
  var x_101 : bool;
  var x_102_phi : bool;
  let x_39 : f32 = x_6.x_GLF_uniform_float_values[1];
  A[0] = x_39;
  let x_42 : f32 = x_6.x_GLF_uniform_float_values[1];
  A[1] = x_42;
  let x_45 : i32 = x_9.x_GLF_uniform_int_values[0];
  i = x_45;
  loop {
    let x_50 : i32 = i;
    let x_52 : i32 = x_9.x_GLF_uniform_int_values[3];
    if ((x_50 < x_52)) {
    } else {
      break;
    }
    let x_56 : i32 = x_9.x_GLF_uniform_int_values[0];
    j = x_56;
    loop {
      let x_61 : i32 = j;
      let x_63 : i32 = x_9.x_GLF_uniform_int_values[2];
      if ((x_61 < x_63)) {
      } else {
        break;
      }
      let x_66 : i32 = j;
      switch(x_66) {
        case 1: {
          let x_78 : i32 = i;
          let x_80 : f32 = x_6.x_GLF_uniform_float_values[0];
          A[x_78] = x_80;
        }
        case 0: {
          let x_70 : i32 = i;
          if ((-2147483648 < x_70)) {
            continue;
          }
          let x_74 : i32 = i;
          let x_76 : f32 = x_6.x_GLF_uniform_float_values[2];
          A[x_74] = x_76;
        }
        default: {
        }
      }

      continuing {
        let x_82 : i32 = j;
        j = (x_82 + 1);
      }
    }

    continuing {
      let x_84 : i32 = i;
      i = (x_84 + 1);
    }
  }
  let x_87 : i32 = x_9.x_GLF_uniform_int_values[0];
  let x_89 : f32 = A[x_87];
  let x_91 : f32 = x_6.x_GLF_uniform_float_values[0];
  let x_92 : bool = (x_89 == x_91);
  x_102_phi = x_92;
  if (x_92) {
    let x_96 : i32 = x_9.x_GLF_uniform_int_values[1];
    let x_98 : f32 = A[x_96];
    let x_100 : f32 = x_6.x_GLF_uniform_float_values[0];
    x_101 = (x_98 == x_100);
    x_102_phi = x_101;
  }
  let x_102 : bool = x_102_phi;
  if (x_102) {
    let x_107 : i32 = x_9.x_GLF_uniform_int_values[1];
    let x_110 : i32 = x_9.x_GLF_uniform_int_values[0];
    let x_113 : i32 = x_9.x_GLF_uniform_int_values[0];
    let x_116 : i32 = x_9.x_GLF_uniform_int_values[1];
    x_GLF_color = vec4<f32>(f32(x_107), f32(x_110), f32(x_113), f32(x_116));
  } else {
    let x_120 : i32 = x_9.x_GLF_uniform_int_values[1];
    let x_121 : f32 = f32(x_120);
    x_GLF_color = vec4<f32>(x_121, x_121, x_121, x_121);
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
