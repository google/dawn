[[block]]
struct buf1 {
  injectionSwitch : vec2<f32>;
};

[[block]]
struct buf2 {
  resolution : vec2<f32>;
};

type RTArr = [[stride(4)]] array<i32>;

[[block]]
struct doesNotMatter {
  x_compute_data : RTArr;
};

var<private> gl_GlobalInvocationID : vec3<u32>;

[[group(0), binding(1)]] var<uniform> x_10 : buf1;

[[group(0), binding(2)]] var<uniform> x_13 : buf2;

[[group(0), binding(0)]] var<storage, read_write> x_15 : doesNotMatter;

fn main_1() {
  var A : array<f32, 1>;
  var i : i32;
  var value : vec4<f32>;
  var m : i32;
  var l : i32;
  var n : i32;
  A[0] = 0.0;
  i = 0;
  loop {
    let x_60 : i32 = i;
    if ((x_60 < 50)) {
    } else {
      break;
    }
    let x_63 : i32 = i;
    if ((x_63 > 0)) {
      let x_68 : f32 = A[0];
      let x_70 : f32 = A[0];
      A[0] = (x_70 + x_68);
    }

    continuing {
      let x_73 : i32 = i;
      i = (x_73 + 1);
    }
  }
  loop {
    let x_80 : u32 = gl_GlobalInvocationID.x;
    if ((x_80 < 100u)) {
      value = vec4<f32>(0.0, 0.0, 0.0, 1.0);
      m = 0;
      loop {
        let x_89 : i32 = m;
        if ((x_89 < 1)) {
        } else {
          break;
        }
        l = 0;
        loop {
          let x_96 : i32 = l;
          if ((x_96 < 1)) {
          } else {
            break;
          }
          let x_100 : f32 = x_10.injectionSwitch.x;
          let x_102 : f32 = x_10.injectionSwitch.y;
          if ((x_100 > x_102)) {
            return;
          }

          continuing {
            let x_106 : i32 = l;
            l = (x_106 + 1);
          }
        }

        continuing {
          let x_108 : i32 = m;
          m = (x_108 + 1);
        }
      }
      n = 0;
      loop {
        let x_114 : i32 = n;
        if ((x_114 < 1)) {
        } else {
          break;
        }
        let x_118 : f32 = x_10.injectionSwitch.x;
        let x_120 : f32 = x_10.injectionSwitch.y;
        if ((x_118 > x_120)) {
          workgroupBarrier();
        }

        continuing {
          let x_124 : i32 = n;
          n = (x_124 + 1);
        }
      }
    } else {
      let x_127 : u32 = gl_GlobalInvocationID.x;
      if ((x_127 < 120u)) {
        let x_133 : f32 = A[0];
        let x_135 : f32 = x_13.resolution.x;
        let x_138 : f32 = A[0];
        let x_140 : f32 = x_13.resolution.y;
        value = vec4<f32>((x_133 / x_135), (x_138 / x_140), 0.0, 1.0);
      } else {
        let x_144 : f32 = x_10.injectionSwitch.x;
        let x_146 : f32 = x_10.injectionSwitch.y;
        if ((x_144 > x_146)) {
          continue;
        }
      }
    }

    continuing {
      if (false) {
      } else {
        break;
      }
    }
  }
  let x_151 : f32 = value.x;
  x_15.x_compute_data[0] = i32(x_151);
  let x_155 : f32 = value.y;
  x_15.x_compute_data[1] = i32(x_155);
  let x_159 : f32 = value.z;
  x_15.x_compute_data[2] = i32(x_159);
  let x_163 : f32 = value.w;
  x_15.x_compute_data[3] = i32(x_163);
  return;
}

[[stage(compute), workgroup_size(1, 1, 1)]]
fn main([[builtin(global_invocation_id)]] gl_GlobalInvocationID_param : vec3<u32>) {
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}
