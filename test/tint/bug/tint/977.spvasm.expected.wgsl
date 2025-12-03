struct ResultMatrix {
  numbers : array<f32>,
}

@group(0u) @binding(2u) var<storage, read_write> resultMatrix : ResultMatrix;

struct FirstMatrix {
  numbers : array<f32>,
}

@group(0u) @binding(0u) var<storage, read> firstMatrix : FirstMatrix;

struct SecondMatrix {
  numbers : array<f32>,
}

@group(0u) @binding(1u) var<storage, read> secondMatrix : SecondMatrix;

struct Uniforms {
  NAN : f32,
  sizeA : i32,
  sizeB : i32,
}

@group(0u) @binding(3u) var<uniform> v : Uniforms;

@compute @workgroup_size(1u, 1u, 1u)
fn main(@builtin(global_invocation_id) gl_GlobalInvocationID : vec3<u32>) {
  var index : i32;
  var a : i32;
  var param : f32;
  var param_1 : f32;
  index = bitcast<i32>(gl_GlobalInvocationID.x);
  a = -10i;
  let v_1 = index;
  param = -4.0f;
  param_1 = -3.0f;
  resultMatrix.numbers[v_1] = v_2(&(param), &(param_1));
}

fn v_2(a : ptr<function, f32>, b : ptr<function, f32>) -> f32 {
  var v_3 : f32;
  if ((*(b) == 0.0f)) {
    return 1.0f;
  }
  let v_4 = *(b);
  if (!((round((v_4 - (2.0f * floor((v_4 / 2.0f))))) == 1.0f))) {
    v_3 = pow(abs(*(a)), *(b));
  } else {
    v_3 = (sign(*(a)) * pow(abs(*(a)), *(b)));
  }
  return v_3;
}
