// flags:  --hlsl-shader-model 62
enable f16;
var<private> u : mat2x2<f16> = mat2x2<f16>(mat2x2<f32>(1.0f, 2.0f,
                                                       3.0f, 4.0f));