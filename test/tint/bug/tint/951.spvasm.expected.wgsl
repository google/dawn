type RTArr = array<f32>;

type RTArr_1 = array<f32>;

struct ssbOut {
  result : RTArr_1,
}

struct ssbA {
  A : RTArr_1,
}

struct Uniforms {
  NAN : f32,
  aShape : i32,
  outShape : i32,
  outShapeStrides : i32,
  size : i32,
}

@group(0) @binding(0) var<storage, read_write> x_16 : ssbOut;

@group(0) @binding(1) var<storage, read> x_20 : ssbA;

var<private> gl_GlobalInvocationID : vec3<u32>;

@group(0) @binding(2) var<uniform> x_24 : Uniforms;

fn getAAtOutCoords_() -> f32 {
  let x_42 : u32 = gl_GlobalInvocationID.x;
  let x_44 : f32 = x_20.A[x_42];
  return x_44;
}

fn unaryOperation_f1_(a : ptr<function, f32>) -> f32 {
  let x_47 : f32 = *(a);
  if ((x_47 < 0.0)) {
    return 0x1p+128;
  }
  let x_55 : f32 = *(a);
  return log(x_55);
}

fn setOutput_i1_f1_(flatIndex : ptr<function, i32>, value : ptr<function, f32>) {
  let x_27 : i32 = *(flatIndex);
  let x_28 : f32 = *(value);
  x_16.result[x_27] = x_28;
  return;
}

fn main_1() {
  var index : i32;
  var a_1 : f32;
  var param : f32;
  var param_1 : i32;
  var param_2 : f32;
  let x_61 : u32 = gl_GlobalInvocationID.x;
  index = bitcast<i32>(x_61);
  let x_63 : i32 = index;
  let x_70 : i32 = x_24.size;
  if ((x_63 < x_70)) {
    let x_75 : f32 = getAAtOutCoords_();
    a_1 = x_75;
    let x_77 : f32 = a_1;
    param = x_77;
    let x_78 : f32 = unaryOperation_f1_(&(param));
    let x_80 : i32 = index;
    param_1 = x_80;
    param_2 = x_78;
    setOutput_i1_f1_(&(param_1), &(param_2));
  }
  return;
}

@stage(compute) @workgroup_size(128i, 1i, 1i)
fn main(@builtin(global_invocation_id) gl_GlobalInvocationID_param : vec3<u32>) {
  gl_GlobalInvocationID = gl_GlobalInvocationID_param;
  main_1();
}
