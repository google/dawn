[[block]]
struct buf0 {
  injectionSwitch : vec2<f32>;
};

type RTArr = [[stride(4)]] array<u32>;

[[block]]
struct doesNotMatter {
  x_compute_data : RTArr;
};

var<private> GLF_live2gl_FragCoord : vec4<f32>;

[[group(0), binding(1)]] var<uniform> x_9 : buf0;

[[group(0), binding(0)]] var<storage, read_write> x_12 : doesNotMatter;

fn main_1() {
  var GLF_live2_looplimiter1 : i32;
  var i : i32;
  var j : i32;
  var GLF_dead3x : f32;
  var x_51 : f32;
  var GLF_dead3k : i32;
  GLF_live2_looplimiter1 = 0;
  i = 0;
  loop {
    let x_56 : i32 = i;
    if ((x_56 < 1)) {
    } else {
      break;
    }
    let x_59 : i32 = GLF_live2_looplimiter1;
    if ((x_59 >= 3)) {
      j = 0;
      loop {
        let x_67 : i32 = j;
        if ((x_67 < 1)) {
        } else {
          break;
        }
        let x_13 : f32 = GLF_live2gl_FragCoord.x;
        if ((i32(x_13) < 120)) {
        } else {
          workgroupBarrier();
        }

        continuing {
          let x_76 : i32 = j;
          j = (x_76 + 1);
        }
      }
      break;
    }

    continuing {
      let x_78 : i32 = i;
      i = (x_78 + 1);
    }
  }
  let x_81 : f32 = x_9.injectionSwitch.x;
  let x_83 : f32 = x_9.injectionSwitch.y;
  if ((x_81 > x_83)) {
    let x_14 : f32 = GLF_live2gl_FragCoord.x;
    x_51 = x_14;
  } else {
    x_51 = 0.0;
  }
  let x_15 : f32 = x_51;
  GLF_dead3x = x_15;
  GLF_dead3k = 0;
  loop {
    let x_93 : i32 = GLF_dead3k;
    if ((x_93 < 2)) {
    } else {
      break;
    }
    let x_96 : f32 = GLF_dead3x;
    if ((x_96 > 4.0)) {
      break;
    }
    let x_16 : f32 = GLF_live2gl_FragCoord.x;
    GLF_dead3x = x_16;
    workgroupBarrier();

    continuing {
      let x_101 : i32 = GLF_dead3k;
      GLF_dead3k = (x_101 + 1);
    }
  }
  x_12.x_compute_data[0] = 42u;
  return;
}

[[stage(compute), workgroup_size(1, 18, 6)]]
fn main() {
  main_1();
}
