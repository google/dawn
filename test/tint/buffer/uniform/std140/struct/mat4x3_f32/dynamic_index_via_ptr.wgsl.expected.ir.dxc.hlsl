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

Failed to generate: :38:24 error: binary: %23 is not in scope
    %22:u32 = add %21, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:64:5 note: %23 declared here
    %23:u32 = mul %52, 4u
    ^^^^^^^

:43:24 error: binary: %23 is not in scope
    %29:u32 = add %28, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:64:5 note: %23 declared here
    %23:u32 = mul %52, 4u
    ^^^^^^^

:51:24 error: binary: %23 is not in scope
    %39:u32 = add %38, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:64:5 note: %23 declared here
    %23:u32 = mul %52, 4u
    ^^^^^^^

:56:24 error: binary: %23 is not in scope
    %45:u32 = add %44, %23
                       ^^^

:24:3 note: in block
  $B3: {
  ^^^

:64:5 note: %23 declared here
    %23:u32 = mul %52, 4u
    ^^^^^^^

:64:5 error: binary: no matching overload for 'operator * (i32, u32)'

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

    %23:u32 = mul %52, 4u
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
    %20:u32 = add %10, %13
    %21:u32 = add %20, %16
    %22:u32 = add %21, %23
    %24:Outer = call %25, %22
    %l_a_i:Outer = let %24
    %27:u32 = add %10, %13
    %28:u32 = add %27, %16
    %29:u32 = add %28, %23
    %30:array<Inner, 4> = call %31, %29
    %l_a_i_a:array<Inner, 4> = let %30
    %33:u32 = add %10, %13
    %34:Inner = call %35, %33
    %l_a_i_a_i:Inner = let %34
    %37:u32 = add %10, %13
    %38:u32 = add %37, %16
    %39:u32 = add %38, %23
    %40:mat4x3<f32> = call %41, %39
    %l_a_i_a_i_m:mat4x3<f32> = let %40
    %43:u32 = add %10, %13
    %44:u32 = add %43, %16
    %45:u32 = add %44, %23
    %46:u32 = div %45, 16u
    %47:ptr<uniform, vec4<u32>, read> = access %a, %46
    %48:vec4<u32> = load %47
    %49:vec3<u32> = swizzle %48, xyz
    %50:vec3<f32> = bitcast %49
    %l_a_i_a_i_m_i:vec3<f32> = let %50
    %52:i32 = call %i
    %23:u32 = mul %52, 4u
    %53:u32 = add %10, %13
    %54:u32 = add %53, %16
    %55:u32 = add %54, %23
    %56:u32 = div %55, 16u
    %57:ptr<uniform, vec4<u32>, read> = access %a, %56
    %58:u32 = mod %55, 16u
    %59:u32 = div %58, 4u
    %60:u32 = load_vector_element %57, %59
    %61:f32 = bitcast %60
    %l_a_i_a_i_m_i_i:f32 = let %61
    ret
  }
}
%35 = func(%start_byte_offset:u32):Inner {
  $B4: {
    %64:mat4x3<f32> = call %41, %start_byte_offset
    %65:Inner = construct %64
    ret %65
  }
}
%41 = func(%start_byte_offset_1:u32):mat4x3<f32> {  # %start_byte_offset_1: 'start_byte_offset'
  $B5: {
    %67:u32 = div %start_byte_offset_1, 16u
    %68:ptr<uniform, vec4<u32>, read> = access %a, %67
    %69:vec4<u32> = load %68
    %70:vec3<u32> = swizzle %69, xyz
    %71:vec3<f32> = bitcast %70
    %72:u32 = add 16u, %start_byte_offset_1
    %73:u32 = div %72, 16u
    %74:ptr<uniform, vec4<u32>, read> = access %a, %73
    %75:vec4<u32> = load %74
    %76:vec3<u32> = swizzle %75, xyz
    %77:vec3<f32> = bitcast %76
    %78:u32 = add 32u, %start_byte_offset_1
    %79:u32 = div %78, 16u
    %80:ptr<uniform, vec4<u32>, read> = access %a, %79
    %81:vec4<u32> = load %80
    %82:vec3<u32> = swizzle %81, xyz
    %83:vec3<f32> = bitcast %82
    %84:u32 = add 48u, %start_byte_offset_1
    %85:u32 = div %84, 16u
    %86:ptr<uniform, vec4<u32>, read> = access %a, %85
    %87:vec4<u32> = load %86
    %88:vec3<u32> = swizzle %87, xyz
    %89:vec3<f32> = bitcast %88
    %90:mat4x3<f32> = construct %71, %77, %83, %89
    ret %90
  }
}
%31 = func(%start_byte_offset_2:u32):array<Inner, 4> {  # %start_byte_offset_2: 'start_byte_offset'
  $B6: {
    %a_1:ptr<function, array<Inner, 4>, read_write> = var, array<Inner, 4>(Inner(mat4x3<f32>(vec3<f32>(0.0f))))  # %a_1: 'a'
    loop [i: $B7, b: $B8, c: $B9] {  # loop_1
      $B7: {  # initializer
        next_iteration 0u  # -> $B8
      }
      $B8 (%idx:u32): {  # body
        %94:bool = gte %idx, 4u
        if %94 [t: $B10] {  # if_1
          $B10: {  # true
            exit_loop  # loop_1
          }
        }
        %95:u32 = mul %idx, 64u
        %96:u32 = add %start_byte_offset_2, %95
        %97:ptr<function, Inner, read_write> = access %a_1, %idx
        %98:Inner = call %35, %96
        store %97, %98
        continue  # -> $B9
      }
      $B9: {  # continuing
        %99:u32 = add %idx, 1u
        next_iteration %99  # -> $B8
      }
    }
    %100:array<Inner, 4> = load %a_1
    ret %100
  }
}
%25 = func(%start_byte_offset_3:u32):Outer {  # %start_byte_offset_3: 'start_byte_offset'
  $B11: {
    %102:array<Inner, 4> = call %31, %start_byte_offset_3
    %103:Outer = construct %102
    ret %103
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
        %107:bool = gte %idx_1, 4u
        if %107 [t: $B16] {  # if_2
          $B16: {  # true
            exit_loop  # loop_2
          }
        }
        %108:u32 = mul %idx_1, 256u
        %109:u32 = add %start_byte_offset_4, %108
        %110:ptr<function, Outer, read_write> = access %a_2, %idx_1
        %111:Outer = call %25, %109
        store %110, %111
        continue  # -> $B15
      }
      $B15: {  # continuing
        %112:u32 = add %idx_1, 1u
        next_iteration %112  # -> $B14
      }
    }
    %113:array<Outer, 4> = load %a_2
    ret %113
  }
}

