var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_29 : bool = false;
  var x_30 : f32;
  var x_31 : f32;
  var x_32 : f32;
  var x_33 : bool = false;
  var x_34 : f32;
  var x_35 : f32;
  var x_36 : f32;
  var f : f32;
  var i : f32;
  var param : f32;
  var param_1 : f32;
  f = 0.0;
  i = 0.0;
  loop {
    let x_38 : f32 = i;
    param = x_38;
    x_33 = false;
    loop {
      let x_41 : f32 = param;
      x_35 = x_41;
      loop {
        let x_48 : f32 = x_35;
        let x_49 : f32 = param;
        if ((x_48 == x_49)) {
          let x_53 : f32 = x_35;
          x_33 = true;
          x_34 = x_53;
          break;
        }
        let x_54 : f32 = x_35;
        x_35 = (x_54 + 1.0);

        continuing {
          let x_56 : f32 = x_35;
          let x_57 : f32 = param;
          if ((x_56 < x_57)) {
          } else {
            break;
          }
        }
      }
      let x_59 : bool = x_33;
      if (x_59) {
        break;
      }
      x_33 = true;
      x_34 = 0.0;
      break;
    }
    let x_61 : f32 = x_34;
    x_36 = x_61;
    f = x_61;
    param_1 = 1.0;
    x_29 = false;
    loop {
      let x_63 : f32 = param_1;
      x_31 = x_63;
      loop {
        let x_70 : f32 = x_31;
        let x_71 : f32 = param_1;
        if ((x_70 == x_71)) {
          let x_75 : f32 = x_31;
          x_29 = true;
          x_30 = x_75;
          break;
        }
        let x_76 : f32 = x_31;
        x_31 = (x_76 + 1.0);

        continuing {
          let x_78 : f32 = x_31;
          let x_79 : f32 = param_1;
          if ((x_78 < x_79)) {
          } else {
            break;
          }
        }
      }
      let x_81 : bool = x_29;
      if (x_81) {
        break;
      }
      x_29 = true;
      x_30 = 0.0;
      break;
    }
    let x_83 : f32 = x_30;
    x_32 = x_83;
    let x_84 : f32 = i;
    let x_85 : f32 = (x_84 + x_83);
    i = x_85;
    if ((x_85 < 6.0)) {
    } else {
      break;
    }
  }
  let x_87 : f32 = f;
  if ((x_87 == 5.0)) {
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

fn func_f1_(x : ptr<function, f32>) -> f32 {
  var x_93 : bool = false;
  var x_94 : f32;
  var a : f32;
  loop {
    let x_96 : f32 = *(x);
    a = x_96;
    loop {
      let x_103 : f32 = a;
      let x_104 : f32 = *(x);
      if ((x_103 == x_104)) {
        let x_108 : f32 = a;
        x_93 = true;
        x_94 = x_108;
        break;
      }
      let x_109 : f32 = a;
      a = (x_109 + 1.0);

      continuing {
        let x_111 : f32 = a;
        let x_112 : f32 = *(x);
        if ((x_111 < x_112)) {
        } else {
          break;
        }
      }
    }
    let x_114 : bool = x_93;
    if (x_114) {
      break;
    }
    x_93 = true;
    x_94 = 0.0;
    break;
  }
  let x_116 : f32 = x_94;
  return x_116;
}
