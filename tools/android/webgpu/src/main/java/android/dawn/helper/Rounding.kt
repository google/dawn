package android.dawn.helper

fun Long.roundDownToNearestMultipleOf(boundary: Int) = this / boundary * boundary
fun Int.roundDownToNearestMultipleOf(boundary: Int) = this / boundary * boundary
fun Long.roundUpToNearestMultipleOf(boundary: Int) = (this + boundary - 1) / boundary * boundary
fun Int.roundUpToNearestMultipleOf(boundary: Int) = (this + boundary - 1) / boundary * boundary
