// flags:  --hlsl-shader-model 62
enable f16;
var<private> u : mat2x2<f32> = mat2x2<f32>(mat2x2<f16>(1.0h, 2.0h,
                                                       3.0h, 4.0h));