[[block]]
struct Uniforms {
  dimAOuter : u32;
  dimInner : u32;
  dimBOuter : u32;
};

[[block]]
struct Matrix {
  numbers : array<f32>;
};

[[group(0), binding(0)]] var<storage, read> firstMatrix : Matrix;

[[group(0), binding(1)]] var<storage, read> secondMatrix : Matrix;

[[group(0), binding(2)]] var<storage, write> resultMatrix : Matrix;

[[group(0), binding(3)]] var<uniform> uniforms : Uniforms;

fn mm_readA(row : u32, col : u32) -> f32 {
  if (((row < uniforms.dimAOuter) && (col < uniforms.dimInner))) {
    let result : f32 = firstMatrix.numbers[((row * uniforms.dimInner) + col)];
    return result;
  }
  return 0.0;
}

fn mm_readB(row : u32, col : u32) -> f32 {
  if (((row < uniforms.dimInner) && (col < uniforms.dimBOuter))) {
    let result : f32 = secondMatrix.numbers[((row * uniforms.dimBOuter) + col)];
    return result;
  }
  return 0.0;
}

fn mm_write(row : u32, col : u32, value : f32) {
  if (((row < uniforms.dimAOuter) && (col < uniforms.dimBOuter))) {
    let index : u32 = (col + (row * uniforms.dimBOuter));
    resultMatrix.numbers[index] = value;
  }
}

let RowPerThread : u32 = 4u;

let ColPerThread : u32 = 4u;

let TileAOuter : u32 = 64u;

let TileBOuter : u32 = 64u;

let TileInner : u32 = 64u;

var<workgroup> mm_Asub : array<array<f32, 64>, 64>;

var<workgroup> mm_Bsub : array<array<f32, 64>, 64>;

[[stage(compute), workgroup_size(16, 16, 1)]]
fn main([[builtin(local_invocation_id)]] local_id : vec3<u32>, [[builtin(global_invocation_id)]] global_id : vec3<u32>) {
  let tileRow : u32 = (local_id.y * RowPerThread);
  let tileCol : u32 = (local_id.x * ColPerThread);
  let globalRow : u32 = (global_id.y * RowPerThread);
  let globalCol : u32 = (global_id.x * ColPerThread);
  let numTiles : u32 = (((uniforms.dimInner - 1u) / TileInner) + 1u);
  var acc : array<f32, 16>;
  var ACached : f32;
  var BCached : array<f32, 4>;
  {
    var index : u32 = 0u;
    loop {
      if (!((index < (RowPerThread * ColPerThread)))) {
        break;
      }
      acc[index] = 0.0;

      continuing {
        index = (index + 1u);
      }
    }
  }
  let ColPerThreadA : u32 = (TileInner / 16u);
  let tileColA : u32 = (local_id.x * ColPerThreadA);
  let RowPerThreadB : u32 = (TileInner / 16u);
  let tileRowB : u32 = (local_id.y * RowPerThreadB);
  {
    var t : u32 = 0u;
    loop {
      if (!((t < numTiles))) {
        break;
      }
      {
        var innerRow : u32 = 0u;
        loop {
          if (!((innerRow < RowPerThread))) {
            break;
          }
          {
            var innerCol : u32 = 0u;
            loop {
              if (!((innerCol < ColPerThreadA))) {
                break;
              }
              let inputRow : u32 = (tileRow + innerRow);
              let inputCol : u32 = (tileColA + innerCol);
              mm_Asub[inputRow][inputCol] = mm_readA((globalRow + innerRow), ((t * TileInner) + inputCol));

              continuing {
                innerCol = (innerCol + 1u);
              }
            }
          }

          continuing {
            innerRow = (innerRow + 1u);
          }
        }
      }
      {
        var innerRow : u32 = 0u;
        loop {
          if (!((innerRow < RowPerThreadB))) {
            break;
          }
          {
            var innerCol : u32 = 0u;
            loop {
              if (!((innerCol < ColPerThread))) {
                break;
              }
              let inputRow : u32 = (tileRowB + innerRow);
              let inputCol : u32 = (tileCol + innerCol);
              mm_Bsub[innerCol][inputCol] = mm_readB(((t * TileInner) + inputRow), (globalCol + innerCol));

              continuing {
                innerCol = (innerCol + 1u);
              }
            }
          }

          continuing {
            innerRow = (innerRow + 1u);
          }
        }
      }
      workgroupBarrier();
      {
        var k : u32 = 0u;
        loop {
          if (!((k < TileInner))) {
            break;
          }
          {
            var inner : u32 = 0u;
            loop {
              if (!((inner < ColPerThread))) {
                break;
              }
              BCached[inner] = mm_Bsub[k][(tileCol + inner)];

              continuing {
                inner = (inner + 1u);
              }
            }
          }
          {
            var innerRow : u32 = 0u;
            loop {
              if (!((innerRow < RowPerThread))) {
                break;
              }
              ACached = mm_Asub[(tileRow + innerRow)][k];
              {
                var innerCol : u32 = 0u;
                loop {
                  if (!((innerCol < ColPerThread))) {
                    break;
                  }
                  let index : u32 = ((innerRow * ColPerThread) + innerCol);
                  acc[index] = (acc[index] + (ACached * BCached[innerCol]));

                  continuing {
                    innerCol = (innerCol + 1u);
                  }
                }
              }

              continuing {
                innerRow = (innerRow + 1u);
              }
            }
          }

          continuing {
            k = (k + 1u);
          }
        }
      }
      workgroupBarrier();

      continuing {
        t = (t + 1u);
      }
    }
  }
  {
    var innerRow : u32 = 0u;
    loop {
      if (!((innerRow < RowPerThread))) {
        break;
      }
      {
        var innerCol : u32 = 0u;
        loop {
          if (!((innerCol < ColPerThread))) {
            break;
          }
          let index : u32 = ((innerRow * ColPerThread) + innerCol);
          mm_write((globalRow + innerRow), (globalCol + innerCol), acc[index]);

          continuing {
            innerCol = (innerCol + 1u);
          }
        }
      }

      continuing {
        innerRow = (innerRow + 1u);
      }
    }
  }
}
