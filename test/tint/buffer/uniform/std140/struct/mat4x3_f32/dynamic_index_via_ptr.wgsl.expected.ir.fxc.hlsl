SKIP: FAILED


struct Inner {
  @size(64)
  m : mat4x3<f32>,
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
  let l_a_i_a_i_m : mat4x3<f32> = *(p_a_i_a_i_m);
  let l_a_i_a_i_m_i : vec3<f32> = *(p_a_i_a_i_m_i);
  let l_a_i_a_i_m_i_i : f32 = (*(p_a_i_a_i_m_i))[i()];
}

Failed to generate: :55:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %43:u32 = mul %42, 4u
    ^^^^^^^^^^^^^^^^^^^^^

:24:3 note: in block
  $B3: {
  ^^^

note: # Disassembly
Inner = struct @align(16) {
  m:mat4x3<f32> @offset(0)
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
    %23:array<Inner, 4> = call %24, %10
    %l_a_i_a:array<Inner, 4> = let %23
    %26:u32 = add %10, %13
    %27:Inner = call %28, %26
    %l_a_i_a_i:Inner = let %27
    %30:u32 = add %10, %13
    %31:mat4x3<f32> = call %32, %30
    %l_a_i_a_i_m:mat4x3<f32> = let %31
    %34:u32 = add %10, %13
    %35:u32 = add %34, %16
    %36:u32 = div %35, 16u
    %37:ptr<uniform, vec4<u32>, read> = access %a, %36
    %38:vec4<u32> = load %37
    %39:vec3<u32> = swizzle %38, xyz
    %40:vec3<f32> = bitcast %39
    %l_a_i_a_i_m_i:vec3<f32> = let %40
    %42:i32 = call %i
    %43:u32 = mul %42, 4u
    %44:u32 = add %10, %13
    %45:u32 = add %44, %16
    %46:u32 = add %45, %43
    %47:u32 = div %46, 16u
    %48:ptr<uniform, vec4<u32>, read> = access %a, %47
    %49:u32 = mod %46, 16u
    %50:u32 = div %49, 4u
    %51:u32 = load_vector_element %48, %50
    %52:f32 = bitcast %51
    %l_a_i_a_i_m_i_i:f32 = let %52
    ret
  }
}
%18 = func(%start_byte_offset:u32):array<Outer, 4> {
  $B4: {
    %a_1:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat4x3<f32>(vec3<f32>(0.0f))))))  # %a_1: 'a'
    loop [i: $B5, b: $B6, c: $B7] {  # loop_1
      $B5: {  # initializer
        next_iteration 0u  # -> $B6
      }
      $B6 (%idx:u32): {  # body
        %57:bool = gte %idx, 4u
        if %57 [t: $B8] {  # if_1
          $B8: {  # true
            exit_loop  # loop_1
          }
        }
        %58:u32 = mul %idx, 256u
        %59:u32 = add %start_byte_offset, %58
        %60:ptr<function, Outer, read_write> = access %a_1, %idx
        %61:Outer = call %21, %59
        store %60, %61
        continue  # -> $B7
      }
      $B7: {  # continuing
        %62:u32 = add %idx, 1u
        next_iteration %62  # -> $B6
      }
    }
    %63:array<Outer, 4> = load %a_1
    ret %63
  }
}
%21 = func(%start_byte_offset_1:u32):Outer {  # %start_byte_offset_1: 'start_byte_offset'
  $B9: {
    %65:array<Inner, 4> = call %24, %start_byte_offset_1
    %66:Outer = construct %65
    ret %66
  }
}
%24 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B10: {
    %a_2:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x3<f32>(vec3<f32>(0.0f))))  # %a_2: 'a'
    loop [i: $B11, b: $B12, c: $B13] {  # loop_2
      $B11: {  # initializer
        next_iteration 0u  # -> $B12
      }
      $B12 (%idx_1:u32): {  # body
        %70:bool = gte %idx_1, 4u
        if %70 [t: $B14] {  # if_2
          $B14: {  # true
            exit_loop  # loop_2
          }
        }
        %71:u32 = mul %idx_1, 64u
        %72:u32 = add %start_byte_offset_2, %71
        %73:ptr<function, Inner, read_write> = access %a_2, %idx_1
        %74:Inner = call %28, %72
        store %73, %74
        continue  # -> $B13
      }
      $B13: {  # continuing
        %75:u32 = add %idx_1, 1u
        next_iteration %75  # -> $B12
      }
    }
    %76:array<Inner, 4> = load %a_2
    ret %76
  }
}
%28 = func(%start_byte_offset_3:u32):Inner {  # %start_byte_offset_3: 'start_byte_offset'
  $B15: {
    %78:mat4x3<f32> = call %32, %start_byte_offset_3
    %79:Inner = construct %78
    ret %79
  }
}
%32 = func(%start_byte_offset_4:u32):mat4x3<f32> {  # %start_byte_offset_4: 'start_byte_offset'
  $B16: {
    %81:u32 = div %start_byte_offset_4, 16u
    %82:ptr<uniform, vec4<u32>, read> = access %a, %81
    %83:vec4<u32> = load %82
    %84:vec3<u32> = swizzle %83, xyz
    %85:vec3<f32> = bitcast %84
    %86:u32 = add 16u, %start_byte_offset_4
    %87:u32 = div %86, 16u
    %88:ptr<uniform, vec4<u32>, read> = access %a, %87
    %89:vec4<u32> = load %88
    %90:vec3<u32> = swizzle %89, xyz
    %91:vec3<f32> = bitcast %90
    %92:u32 = add 32u, %start_byte_offset_4
    %93:u32 = div %92, 16u
    %94:ptr<uniform, vec4<u32>, read> = access %a, %93
    %95:vec4<u32> = load %94
    %96:vec3<u32> = swizzle %95, xyz
    %97:vec3<f32> = bitcast %96
    %98:u32 = add 48u, %start_byte_offset_4
    %99:u32 = div %98, 16u
    %100:ptr<uniform, vec4<u32>, read> = access %a, %99
    %101:vec4<u32> = load %100
    %102:vec3<u32> = swizzle %101, xyz
    %103:vec3<f32> = bitcast %102
    %104:mat4x3<f32> = construct %85, %91, %97, %103
    ret %104
  }
}

