SKIP: FAILED

VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

%compute_main = @compute @workgroup_size(1i, 1i, 1i) func(%wgid:vec3<u32> [@workgroup_id]):void {
  $B1: {
    %a:i32 = let 10i
    %b:u32 = let 20u
    %c:f32 = let 30.5f
    %6:string = format_string "single: ", %a
    %7:void = print %6
    %8:string = format_string "multiple: a=", %a, ", b=", %b, ", c=", %c
    %9:void = print %8
    %10:i32 = add %a, 5i
    %11:string = format_string "expr: ", %10
    %12:void = print %11
    %13:string = format_string "adjacent: ", %a, %b
    %14:void = print %13
    %15:u32 = access %wgid, 0u
    %16:string = format_string "percent: 100% complete: ", %15
    %17:void = print %16
    ret
  }
}
%fragment_main = @fragment func():void {
  $B2: {
    %19:string = format_string "frag: ", vec4<u32>(1u, 2u, 3u, 4u)
    %20:void = print %19
    ret
  }
}
%vertex_main = @vertex func():VertexOutput {
  $B3: {
    %out:ptr<function, VertexOutput, read_write> = var undef
    %23:ptr<function, vec4<f32>, read_write> = access %out, 0u
    store %23, vec4<f32>(0.0f)
    %24:string = format_string "vert: ", 42i
    %25:void = print %24
    %26:VertexOutput = load %out
    ret %26
  }
}
Failed to generate: print is not supported by the HLSL backend
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

%compute_main = @compute @workgroup_size(1i, 1i, 1i) func(%wgid:vec3<u32> [@workgroup_id]):void {
  $B1: {
    %a:i32 = let 10i
    %b:u32 = let 20u
    %c:f32 = let 30.5f
    %6:string = format_string "single: ", %a
    %7:void = print %6
    %8:string = format_string "multiple: a=", %a, ", b=", %b, ", c=", %c
    %9:void = print %8
    %10:i32 = add %a, 5i
    %11:string = format_string "expr: ", %10
    %12:void = print %11
    %13:string = format_string "adjacent: ", %a, %b
    %14:void = print %13
    %15:u32 = access %wgid, 0u
    %16:string = format_string "percent: 100% complete: ", %15
    %17:void = print %16
    ret
  }
}
%fragment_main = @fragment func():void {
  $B2: {
    %19:string = format_string "frag: ", vec4<u32>(1u, 2u, 3u, 4u)
    %20:void = print %19
    ret
  }
}
%vertex_main = @vertex func():VertexOutput {
  $B3: {
    %out:ptr<function, VertexOutput, read_write> = var undef
    %23:ptr<function, vec4<f32>, read_write> = access %out, 0u
    store %23, vec4<f32>(0.0f)
    %24:string = format_string "vert: ", 42i
    %25:void = print %24
    %26:VertexOutput = load %out
    ret %26
  }
}
Failed to generate: print is not supported by the HLSL backend
VertexOutput = struct @align(16) {
  pos:vec4<f32> @offset(0), @builtin(position)
}

%compute_main = @compute @workgroup_size(1i, 1i, 1i) func(%wgid:vec3<u32> [@workgroup_id]):void {
  $B1: {
    %a:i32 = let 10i
    %b:u32 = let 20u
    %c:f32 = let 30.5f
    %6:string = format_string "single: ", %a
    %7:void = print %6
    %8:string = format_string "multiple: a=", %a, ", b=", %b, ", c=", %c
    %9:void = print %8
    %10:i32 = add %a, 5i
    %11:string = format_string "expr: ", %10
    %12:void = print %11
    %13:string = format_string "adjacent: ", %a, %b
    %14:void = print %13
    %15:u32 = access %wgid, 0u
    %16:string = format_string "percent: 100% complete: ", %15
    %17:void = print %16
    ret
  }
}
%fragment_main = @fragment func():void {
  $B2: {
    %19:string = format_string "frag: ", vec4<u32>(1u, 2u, 3u, 4u)
    %20:void = print %19
    ret
  }
}
%vertex_main = @vertex func():VertexOutput {
  $B3: {
    %out:ptr<function, VertexOutput, read_write> = var undef
    %23:ptr<function, vec4<f32>, read_write> = access %out, 0u
    store %23, vec4<f32>(0.0f)
    %24:string = format_string "vert: ", 42i
    %25:void = print %24
    %26:VertexOutput = load %out
    ret %26
  }
}
Failed to generate: print is not supported by the HLSL backend
//
// compute_main
//
//
// fragment_main
//
//
// vertex_main
//

tint executable returned error: exit status 1
