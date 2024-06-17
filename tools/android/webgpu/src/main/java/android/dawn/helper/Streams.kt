package android.dawn.helper

import java.io.InputStream
import java.io.InputStreamReader
import java.nio.charset.StandardCharsets
import java.util.Scanner

fun streamToString(inputStream: InputStream): String {
    val scanner = Scanner(
        InputStreamReader(
            inputStream,
            StandardCharsets.UTF_8
        )
    ).useDelimiter("\\A")
    return if (scanner.hasNext()) scanner.next() else ""
}

