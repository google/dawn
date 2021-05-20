[[block]]
struct Uniforms {
  aShape : vec2<u32>;
  bShape : vec2<u32>;
  outShape : vec2<u32>;
};

[[block]]
struct Matrix {
  numbers : array<u32>;
};

[[group(0), binding(0)]] var<storage> firstMatrix : [[access(read)]] Matrix;

[[group(0), binding(1)]] var<storage> secondMatrix : [[access(read)]] Matrix;

[[group(0), binding(2)]] var<storage> resultMatrix : [[access(write)]] Matrix;

[[group(0), binding(3)]] var<uniform> uniforms : Uniforms;

[[stage(compute), workgroup_size(2, 2, 1)]]
fn main([[builtin(global_invocation_id)]] global_id : vec3<u32>) {
  let resultCell : vec2<u32> = vec2<u32>(global_id.y, global_id.x);
  let dimInner : u32 = uniforms.aShape.y;
  let dimOutter : u32 = uniforms.outShape.y;
  var result : u32 = 0u;
  {
    var i : u32 = 0u;
    loop {
      if (!((i < dimInner))) {
        break;
      }
      let a : u32 = (i + (resultCell.x * dimInner));
      let b : u32 = (resultCell.y + (i * dimOutter));
      result = (result + (firstMatrix.numbers[a] * secondMatrix.numbers[b]));

      continuing {
        i = (i + 1u);
      }
    }
  }
  let index : u32 = (resultCell.y + (resultCell.x * dimOutter));
  resultMatrix.numbers[index] = result;
}
