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
%32 = func(%start_byte_offset:u32):mat4x3<f32> {
  $B4: {
    %55:u32 = div %start_byte_offset, 16u
    %56:ptr<uniform, vec4<u32>, read> = access %a, %55
    %57:vec4<u32> = load %56
    %58:vec3<u32> = swizzle %57, xyz
    %59:vec3<f32> = bitcast %58
    %60:u32 = add 16u, %start_byte_offset
    %61:u32 = div %60, 16u
    %62:ptr<uniform, vec4<u32>, read> = access %a, %61
    %63:vec4<u32> = load %62
    %64:vec3<u32> = swizzle %63, xyz
    %65:vec3<f32> = bitcast %64
    %66:u32 = add 32u, %start_byte_offset
    %67:u32 = div %66, 16u
    %68:ptr<uniform, vec4<u32>, read> = access %a, %67
    %69:vec4<u32> = load %68
    %70:vec3<u32> = swizzle %69, xyz
    %71:vec3<f32> = bitcast %70
    %72:u32 = add 48u, %start_byte_offset
    %73:u32 = div %72, 16u
    %74:ptr<uniform, vec4<u32>, read> = access %a, %73
    %75:vec4<u32> = load %74
    %76:vec3<u32> = swizzle %75, xyz
    %77:vec3<f32> = bitcast %76
    %78:mat4x3<f32> = construct %59, %65, %71, %77
    ret %78
  }
}
%28 = func(%start_byte_offset_1:u32):Inner {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %80:mat4x3<f32> = call %32, %start_byte_offset_1
    %81:Inner = construct %80
    ret %81
  }
}
%24 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x3<f32>(vec3<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %85:bool = gte %idx, 4u
        if %85 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %86:u32 = mul %idx, 64u
        %87:u32 = add %start_byte_offset_2, %86
        %88:ptr<function, Inner, read_write> = access %a_1, %idx
        %89:Inner = call %28, %87
        store %88, %89
        continue  # -> $B9
      }
      $B9: {  # continuing
        %90:u32 = add %idx, 1u
        next_iteration %90  # -> $B8
      }
    }
    %91:array<Inner, 4> = load %a_1
    ret %91
  }
}
%21 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %93:array<Inner, 4> = call %24, %start_byte_offset_3
    %94:Outer = construct %93
    ret %94
  }
}
%18 = func(%start_byte_offset_4:u32):array<Outer, 4> {  # %start_byte_offset_4: 'start_byte_offset'
  $B12: {
    %a_2:ptr<function, array<Outer, 4>, read_write> = var, array<Outer, 4>(Outer(array<Inner, 4>(Inner(mat4x3<f32>(vec3<f32>(0.0f))))))  # %a_2: 'a'
    loop [i: $B13, b: $B14, c: $B15] {  # loop_2
      $B13: {  # initializer
        next_iteration 0u  # -> $B14
      }
      $B14 (%idx_1:u32): {  # body
        %98:bool = gte %idx_1, 4u
        if %98 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %99:u32 = mul %idx_1, 256u
        %100:u32 = add %start_byte_offset_4, %99
        %101:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %102:Outer = call %21, %100
        store %101, %102
        continue  # -> $B15
      }
      $B15: {  # continuing
        %103:u32 = add %idx_1, 1u
        next_iteration %103  # -> $B14
      }
    }
    %104:array<Outer, 4> = load %a_2
    ret %104
  }
}

