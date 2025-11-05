package androidx.webgpu.helper

import androidx.webgpu.Status

public class DawnException(public val reason: String = "", public val status: Int? = null) :
    Exception(
        (if (status != null) "${Status.toString(status)}:" else "") + reason
    ) {

}