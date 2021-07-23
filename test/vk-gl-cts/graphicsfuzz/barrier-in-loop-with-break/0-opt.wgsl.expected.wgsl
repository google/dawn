type RTArr = [[stride(4)]] array<i32>;

[[block]]
struct doesNotMatter {
  global_seed : i32;
  data : RTArr;
};

[[block]]
struct buf1 {
  injectionSwitch : vec2<f32>;
};

var<private> gl_LocalInvocationID : vec3<u32>;

[[group(0), binding(0)]] var<storage, read_write> x_7 : doesNotMatter;

[[group(0), binding(1)]] var<uniform> x_10 : buf1;

fn main_1() {
  var lid : i32;
  var val : i32;
  var i : i32;
  let x_40 : u32 = gl_LocalInvocationID.x;
  lid = bitcast<i32>(x_40);
  let x_43 : i32 = x_7.global_seed;
  val = x_43;
  i = 0;
  loop {
    let x_48 : i32 = i;
    if ((x_48 < 2)) {
    } else {
      break;
    }
    let x_51 : i32 = lid;
    if ((x_51 > 0)) {
      let x_55 : i32 = lid;
      let x_58 : i32 = x_7.data[(x_55 - 1)];
      let x_59 : i32 = val;
      val = (x_59 + x_58);
      let x_62 : f32 = x_10.injectionSwitch.x;
      if ((x_62 > 100.0)) {
        break;
      }
    }
    workgroupBarrier();

    continuing {
      let x_66 : i32 = i;
      i = (x_66 + 1);
    }
  }
  let x_68 : i32 = lid;
  if ((x_68 == 0)) {
    x_7.data[0] = 42;
  }
  return;
}

[[stage(compute), workgroup_size(16, 1, 1)]]
fn main([[builtin(local_invocation_id)]] gl_LocalInvocationID_param : vec3<u32>) {
  gl_LocalInvocationID = gl_LocalInvocationID_param;
  main_1();
}
