SKIP: FAILED


enable f16;

struct Inner {
  @size(64)
  m : mat3x3<f16>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

var<private> counter = 0;

fn i() -> i32 {
  counter++;
  return counter;
}

@compute @workgroup_size(1)
fn f() {
  let p_a = &(a);
  let p_a_i = &((*(p_a))[i()]);
  let p_a_i_a = &((*(p_a_i)).a);
  let p_a_i_a_i = &((*(p_a_i_a))[i()]);
  let p_a_i_a_i_m = &((*(p_a_i_a_i)).m);
  let p_a_i_a_i_m_i = &((*(p_a_i_a_i_m))[i()]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_i : Outer = *(p_a_i);
  let l_a_i_a : array<Inner, 4> = *(p_a_i_a);
  let l_a_i_a_i : Inner = *(p_a_i_a_i);
  let l_a_i_a_i_m : mat3x3<f16> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec3<f16> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f16 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:61:5 note: %23 declared here
    %23:u32 = mul %49, 2u
    ^^^^^^^

:48:24 error: binary: %23 is not in scope
    %36:u32 = add %35, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:61:5 note: %23 declared here
    %23:u32 = mul %49, 2u
    ^^^^^^^

:53:24 error: binary: %23 is not in scope
    %42:u32 = add %41, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:61:5 note: %23 declared here
    %23:u32 = mul %49, 2u
    ^^^^^^^

:61:5 error: binary: no matching overload for 'operator * (i32, u32)'

9 candidate operators:
 • 'operator * (T  ✓ , T  ✗ ) -> T' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (vecN<T>  ✗ , T  ✓ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✓ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✓  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (T  ✗ , matNxM<T>  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matNxM<T>  ✗ , T  ✗ ) -> matNxM<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecN<T>  ✗ , vecN<T>  ✗ ) -> vecN<T>' where:
      ✗  'T' is 'f32', 'i32', 'u32' or 'f16'
 • 'operator * (matCxR<T>  ✗ , vecC<T>  ✗ ) -> vecR<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (vecR<T>  ✗ , matCxR<T>  ✗ ) -> vecC<T>' where:
      ✗  'T' is 'f32' or 'f16'
 • 'operator * (matKxR<T>  ✗ , matCxK<T>  ✗ ) -> matCxR<T>' where:
      ✗  'T' is 'f32' or 'f16'

    %23:u32 = mul %49, 2u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(8) {
  m:mat3x3<f16> @offset(0)
}

Outer = struct @align(8) {
  a:array<Inner, 4> @offset(0)
}

$B1: {  # root
  %a:ptr<uniform, array<vec4<u32>, 64>, read> = var @binding_point(0, 0)
  %counter:ptr<private, i32, read_write> = var, 0i
}

%i = func():i32 {
  $B2: {
    %4:i32 = load %counter
    %5:i32 = add %4, 1i
    store %counter, %5
    %6:i32 = load %counter
    ret %6
  }
}
%f = @compute @workgroup_size(1, 1, 1) func():void {
  $B3: {
    %8:i32 = call %i
    %9:u32 = convert %8
    %10:u32 = mul 256u, %9
    %11:i32 = call %i
    %12:u32 = convert %11
    %13:u32 = mul 64u, %12
    %14:i32 = call %i
    %15:u32 = convert %14
    %16:u32 = mul 8u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:u32 = add %10, %13
    %21:u32 = add %20, %16
    %22:u32 = add %21, %23
    %24:Outer = call %25, %22
    %l_a_i:Outer = let %24
    %27:array<Inner, 4> = call %28, %10
    %l_a_i_a:array<Inner, 4> = let %27
    %30:u32 = add %10, %13
    %31:Inner = call %32, %30
    %l_a_i_a_i:Inner = let %31
    %34:u32 = add %10, %13
    %35:u32 = add %34, %16
    %36:u32 = add %35, %23
    %37:mat3x3<f16> = call %38, %36
    %l_a_i_a_i_m:mat3x3<f16> = let %37
    %40:u32 = add %10, %13
    %41:u32 = add %40, %16
    %42:u32 = add %41, %23
    %43:u32 = div %42, 16u
    %44:ptr<uniform, vec4<u32>, read> = access %a, %43
    %45:vec4<u32> = load %44
    %46:vec4<f16> = bitcast %45
    %47:vec3<f16> = swizzle %46, xyz
    %l_a_i_a_i_m_i:vec3<f16> = let %47
    %49:i32 = call %i
    %23:u32 = mul %49, 2u
    %50:u32 = add %10, %13
    %51:u32 = add %50, %16
    %52:u32 = add %51, %23
    %53:u32 = div %52, 16u
    %54:ptr<uniform, vec4<u32>, read> = access %a, %53
    %55:u32 = mod %52, 16u
    %56:u32 = div %55, 4u
    %57:u32 = load_vector_element %54, %56
    %58:u32 = mod %52, 4u
    %59:bool = eq %58, 0u
    %60:u32 = hlsl.ternary 16u, 0u, %59
    %61:u32 = shr %57, %60
    %62:f32 = hlsl.f16tof32 %61
    %63:f16 = convert %62
    %l_a_i_a_i_m_i_i:f16 = let %63
    ret
  }
}
%28 = func(%start_byte_offset:u32):array<Inner, 4> {
  $B4: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat3x3<f16>(vec3<f16>(0.0h))))  # %a_1: 'a'
    loop [i: $B5, b: $B6, c: $B7] {  # loop_1
      $B5: {  # initializer
        next_iteration 0u  # -> $B6
      }
      $B6 (%idx:u32): {  # body
        %68:bool = gte %idx, 4u
        if %68 [t: $B8] {  # if_1
          $B8: {  # true
            exit_loop  # loop_1
          }
        }
        %69:u32 = mul %idx, 64u
        %70:u32 = add %start_byte_offset, %69
        %71:ptr<function, Inner, read_write> = access %a_1, %idx
        %72:Inner = call %32, %70
        store %71, %72
        continue  # -> $B7
      }
      $B7: {  # continuing
        %73:u32 = add %idx, 1u
        next_iteration %73  # -> $B6
      }
    }
    %74:array<Inner, 4> = load %a_1
    ret %74
  }
}
%32 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B9: {
    %76:mat3x3<f16> = call %38, %start_byte_offset_1
    %77:Inner = construct %76
    ret %77
  }
}
%38 = func(%start_byte_offset_2:u32):mat3x3<f16> {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %79:u32 = div %start_byte_offset_2, 16u
    %80:ptr<uniform, vec4<u32>, read> = access %a, %79
    %81:vec4<u32> = load %80
    %82:vec4<f16> = bitcast %81
    %83:vec3<f16> = swizzle %82, xyz
    %84:u32 = add 8u, %start_byte_offset_2
    %85:u32 = div %84, 16u
    %86:ptr<uniform, vec4<u32>, read> = access %a, %85
    %87:vec4<u32> = load %86
    %88:vec4<f16> = bitcast %87
    %89:vec3<f16> = swizzle %88, xyz
    %90:u32 = add 16u, %start_byte_offset_2
    %91:u32 = div %90, 16u
    %92:ptr<uniform, vec4<u32>, read> = access %a, %91
    %93:vec4<u32> = load %92
    %94:vec4<f16> = bitcast %93
    %95:vec3<f16> = swizzle %94, xyz
    %96:mat3x3<f16> = construct %83, %89, %95
    ret %96
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %98:array<Inner, 4> = call %28, %start_byte_offset_3
    %99:Outer = construct %98
    ret %99
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat3x3<f16>(vec3<f16>(0.0h))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %103:bool = gte %idx_1, 4u
        if %103 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %104:u32 = mul %idx_1, 256u
        %105:u32 = add %start_byte_offset_4, %104
        %106:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %107:Outer = call %25, %105
        store %106, %107
        continue  # -> $B15
      }
      $B15: {  # continuing
        %108:u32 = add %idx_1, 1u
        next_iteration %108  # -> $B14
      }
    }
    %109:array<Outer, 4> = load %a_2
    ret %109
  }
}

