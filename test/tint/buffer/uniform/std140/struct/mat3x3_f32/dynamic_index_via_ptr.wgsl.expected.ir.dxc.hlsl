SKIP: FAILED


struct Inner {
  @size(64)
  m : mat3x3<f32>,
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
  let l_a_i_a_i_m : mat3x3<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec3<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :40:24 error: binary: %26 is not in scope
    %25:u32 = add %24, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:60:5 note: %26 declared here
    %26:u32 = mul %48, 4u
    ^^^^^^^

:48:24 error: binary: %26 is not in scope
    %36:u32 = add %35, %26
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:60:5 note: %26 declared here
    %26:u32 = mul %48, 4u
    ^^^^^^^

:60:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %26:u32 = mul %48, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(16) {
  m:mat3x3<f32> @offset(0)
}

Outer = struct @align(16) {
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
    %16:u32 = mul 16u, %15
    %17:array<Outer, 4> = call %18, 0u
    %l_a:array<Outer, 4> = let %17
    %20:Outer = call %21, %10
    %l_a_i:Outer = let %20
    %23:u32 = add %10, %13
    %24:u32 = add %23, %16
    %25:u32 = add %24, %26
    %27:array<Inner, 4> = call %28, %25
    %l_a_i_a:array<Inner, 4> = let %27
    %30:u32 = add %10, %13
    %31:Inner = call %32, %30
    %l_a_i_a_i:Inner = let %31
    %34:u32 = add %10, %13
    %35:u32 = add %34, %16
    %36:u32 = add %35, %26
    %37:mat3x3<f32> = call %38, %36
    %l_a_i_a_i_m:mat3x3<f32> = let %37
    %40:u32 = add %10, %13
    %41:u32 = add %40, %16
    %42:u32 = div %41, 16u
    %43:ptr<uniform, vec4<u32>, read> = access %a, %42
    %44:vec4<u32> = load %43
    %45:vec3<u32> = swizzle %44, xyz
    %46:vec3<f32> = bitcast %45
    %l_a_i_a_i_m_i:vec3<f32> = let %46
    %48:i32 = call %i
    %26:u32 = mul %48, 4u
    %49:u32 = add %10, %13
    %50:u32 = add %49, %16
    %51:u32 = add %50, %26
    %52:u32 = div %51, 16u
    %53:ptr<uniform, vec4<u32>, read> = access %a, %52
    %54:u32 = mod %51, 16u
    %55:u32 = div %54, 4u
    %56:u32 = load_vector_element %53, %55
    %57:f32 = bitcast %56
    %l_a_i_a_i_m_i_i:f32 = let %57
    ret
  }
}
%18 = func(%start_byte_offset:u32):array<Outer, 4> {
  $B4: {
    %a_1:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat3x3<f32>(vec3<f32>(0.0f))))))  # %a_1: 'a'
    loop [i: $B5, b: $B6, c: $B7] {  # loop_1
      $B5: {  # initializer
        next_iteration 0u  # -> $B6
      }
      $B6 (%idx:u32): {  # body
        %62:bool = gte %idx, 4u
        if %62 [t: $B8] {  # if_1
          $B8: {  # true
            exit_loop  # loop_1
          }
        }
        %63:u32 = mul %idx, 256u
        %64:u32 = add %start_byte_offset, %63
        %65:ptr<function, Outer, read_write> = access %a_1, %idx
        %66:Outer = call %21, %64
        store %65, %66
        continue  # -> $B7
      }
      $B7: {  # continuing
        %67:u32 = add %idx, 1u
        next_iteration %67  # -> $B6
      }
    }
    %68:array<Outer, 4> = load %a_1
    ret %68
  }
}
%21 = func(%start_byte_offset_1:u32):Outer {  # %start_byte_offset_1: 'start_byte_offset'
  $B9: {
    %70:array<Inner, 4> = call %28, %start_byte_offset_1
    %71:Outer = construct %70
    ret %71
  }
}
%28 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %a_2:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat3x3<f32>(vec3<f32>(0.0f))))  # %a_2: 'a'
    loop [i: $B11, b: $B12, c: $B13] {  # loop_2
      $B11: {  # initializer
        next_iteration 0u  # -> $B12
      }
      $B12 (%idx_1:u32): {  # body
        %75:bool = gte %idx_1, 4u
        if %75 [t: $B14] {  # if_2
          $B14: {  # true
            exit_loop  # loop_2
          }
        }
        %76:u32 = mul %idx_1, 64u
        %77:u32 = add %start_byte_offset_2, %76
        %78:ptr<function, Inner, read_write> = access %a_2, %idx_1
        %79:Inner = call %32, %77
        store %78, %79
        continue  # -> $B13
      }
      $B13: {  # continuing
        %80:u32 = add %idx_1, 1u
        next_iteration %80  # -> $B12
      }
    }
    %81:array<Inner, 4> = load %a_2
    ret %81
  }
}
%32 = func(%start_byte_offset_3:u32):Inner {  # %start_byte_offset_3: 'start_byte_offset'
  $B15: {
    %83:mat3x3<f32> = call %38, %start_byte_offset_3
    %84:Inner = construct %83
    ret %84
  }
}
%38 = func(%start_byte_offset_4:u32):mat3x3<f32> {  # %start_byte_offset_4: 'start_byte_offset'
  $B16: {
    %86:u32 = div %start_byte_offset_4, 16u
    %87:ptr<uniform, vec4<u32>, read> = access %a, %86
    %88:vec4<u32> = load %87
    %89:vec3<u32> = swizzle %88, xyz
    %90:vec3<f32> = bitcast %89
    %91:u32 = add 16u, %start_byte_offset_4
    %92:u32 = div %91, 16u
    %93:ptr<uniform, vec4<u32>, read> = access %a, %92
    %94:vec4<u32> = load %93
    %95:vec3<u32> = swizzle %94, xyz
    %96:vec3<f32> = bitcast %95
    %97:u32 = add 32u, %start_byte_offset_4
    %98:u32 = div %97, 16u
    %99:ptr<uniform, vec4<u32>, read> = access %a, %98
    %100:vec4<u32> = load %99
    %101:vec3<u32> = swizzle %100, xyz
    %102:vec3<f32> = bitcast %101
    %103:mat3x3<f32> = construct %90, %96, %102
    ret %103
  }
}

