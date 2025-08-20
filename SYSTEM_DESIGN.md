# Android Calculator – System Design (C++ Core + Kotlin UI)

## 1. Goals & Scope
- Simple four-operator calculator (+, -, *, /).
- Android UI kept minimal; all math in C++ for reuse and determinism.
- Safe memory management (RAII), clear JNI boundary, easy to extend.

## 2. Architecture Overview
- Android app (Kotlin) renders UI and handles user interactions.
- JNI bridge exposes a single native function to perform calculation.
- C++ core implements the arithmetic and error handling.
- No background threads (operations are instant); no dynamic allocations needed.

```mermaid
flowchart LR
    UI[Android UI (Kotlin)] -->|primitives via JNI| JNI[JNI Bridge]
    JNI -->|a, op, b| CORE[C++ Calc Core]
    CORE -->|result or error| JNI
    JNI -->|value or exception| UI
```

ASCII sketch (if Mermaid isn’t rendered):

[Kotlin UI] --(a, op, b)--> [JNI] --(a, op, b)--> [C++ Core]
[Kotlin UI] <--(result/exc)--- [JNI] <--(value/err)--- [C++ Core]

## 3. Module Boundaries
- UI (Kotlin): `MainActivity`, XML layout, input validation, button handlers.
- JNI (C++): `native-lib.cpp` exposing `calc(a, op, b)` to Kotlin.
- Core (C++): `calc.cpp/.h` implementing arithmetic with explicit error cases.
- Build: CMake builds a shared lib `libcalc.so` packaged by Gradle.

## 4. File/Package Layout (planned)
- `app/src/main/res/layout/activity_main.xml` – two inputs, four operator button
s, result text.
- `app/src/main/java/com/example/calculator/MainActivity.kt` – UI logic + native
 call.
- `app/src/main/cpp/calc.h` – core header.
- `app/src/main/cpp/calc.cpp` – core implementation.
- `app/src/main/cpp/native-lib.cpp` – JNI glue.
- `app/src/main/cpp/CMakeLists.txt` – lists sources, creates `calc` target, link
s `log`.

## 5. Public APIs (Kotlin ↔ C++)
- Kotlin declaration:
```kotlin
object Native {
    init { System.loadLibrary("calc") }
    external fun calc(a: Double, op: Char, b: Double): Double
}
```
- JNI signature (C++ side):
```cpp
extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_calculator_Native_calc(JNIEnv* env, jclass, jdouble a, jchar op
, jdouble b);
```
- Core function:
```cpp
double calc(double a, char op, double b); // throws on invalid op or divide by z
ero
```

## 6. Error Handling
- Invalid operator → map to `java/lang/IllegalArgumentException`.
- Divide-by-zero → map to `java/lang/ArithmeticException`.
- JNI throws exceptions with `env->ThrowNew(...)` and returns 0 as a dummy value
 (ignored if exception set).
- UI catches exceptions and shows a message; success shows numeric result.

## 7. Memory & Performance
- C++: no heap allocations; primitives only; RAII by default.
- JNI: pass primitives, avoid creating local refs; nothing long-lived.
- Performance is trivial (single CPU ops); no threading needed.

## 8. Minimal UI Behavior
- Two `EditText` fields for operands (numeric keyboard).
- Four `Button`: `+`, `-`, `*`, `/`.
- One `TextView` for result; error toast on exceptions.
- Input validation: empty → treat as error (disable buttons) or default to 0 (ch
oose first option for clarity).

## 9. Build & Tooling
- Create project with Android Studio “Native C++” template (CMake + NDK wired).
- Add `calc.cpp/.h` and wire into `CMakeLists.txt` target `calc`.
- Min SDK: 24+ (adjust as needed). ABI: arm64-v8a; optionally armeabi-v7a, x86_6
4.

## 10. Testing Strategy
- Core unit tests (host or NDK gtest) for `calc` covering normal and error cases
.
- Instrumented UI tests minimal (button taps → result text).
- Use Android Studio Memory Profiler; enable ASan/GWP-ASan in debug for native l
eaks.

## 11. Extensibility
- Add functions for percent, sign toggle, memory (M+, M-, MR) as new ops or dedi
cated JNI calls.
- Switch to Jetpack Compose later without changing native API.
- Internationalization: format output with locale if needed.

## 12. Code Sketches

C++ core (`calc.h` / `calc.cpp`):
```cpp
// calc.h
#pragma once

double calc(double a, char op, double b);
```
```cpp
// calc.cpp
#include "calc.h"
#include <stdexcept>

double calc(double a, char op, double b) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': if (b == 0.0) throw std::runtime_error("division by zero"); re
turn a / b;
        default: throw std::invalid_argument("unknown operator");
    }
}
```

JNI bridge (`native-lib.cpp`):
```cpp
#include <jni.h>
#include <string>
#include "calc.h"

static void throwJava(JNIEnv* env, const char* clazz, const char* msg) {
    jclass ex = env->FindClass(clazz);
    if (ex) env->ThrowNew(ex, msg);
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_calculator_Native_calc(JNIEnv* env, jclass, jdouble a, jchar op
, jdouble b) {
    try {
        return static_cast<jdouble>(calc(static_cast<double>(a), static_cast<cha
r>(op), static_cast<double>(b)));
    } catch (const std::invalid_argument& e) {
        throwJava(env, "java/lang/IllegalArgumentException", e.what());
    } catch (const std::runtime_error& e) {
        throwJava(env, "java/lang/ArithmeticException", e.what());
    } catch (...) {
        throwJava(env, "java/lang/RuntimeException", "unknown error");
    }
    return 0.0; // ignored if exception is pending
}
```

Kotlin UI snippet (XML + Activity):
```xml
<!-- res/layout/activity_main.xml -->
<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    android:orientation="vertical"
    android:padding="16dp">

    <EditText
        android:id="@+id/inputA"
        android:inputType="numberDecimal|numberSigned"
        android:hint="First number"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"/>

    <EditText
        android:id="@+id/inputB"
        android:inputType="numberDecimal|numberSigned"
        android:hint="Second number"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"/>

    <LinearLayout
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:orientation="horizontal"
        android:gravity="center"
        android:layout_marginTop="12dp">
        <Button android:id="@+id/btnAdd" android:text="+"/>
        <Button android:id="@+id/btnSub" android:text="-"/>
        <Button android:id="@+id/btnMul" android:text="*"/>
        <Button android:id="@+id/btnDiv" android:text="/"/>
    </LinearLayout>

    <TextView
        android:id="@+id/txtResult"
        android:textSize="18sp"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:layout_marginTop="12dp"/>
  </LinearLayout>
```
```kotlin
// MainActivity.kt
package com.example.calculator

import android.os.Bundle
import android.widget.*
import androidx.appcompat.app.AppCompatActivity

object Native { init { System.loadLibrary("calc") }; external fun calc(a: Double
, op: Char, b: Double): Double }

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val a = findViewById<EditText>(R.id.inputA)
        val b = findViewById<EditText>(R.id.inputB)
        val result = findViewById<TextView>(R.id.txtResult)

        fun parse(s: String) = s.toDoubleOrNull()
        fun doCalc(op: Char) {
            val av = parse(a.text.toString())
            val bv = parse(b.text.toString())
            if (av == null || bv == null) { Toast.makeText(this, "Enter valid nu
mbers", Toast.LENGTH_SHORT).show(); return }
            try { result.text = Native.calc(av, op, bv).toString() }
            catch (e: IllegalArgumentException) { Toast.makeText(this, e.message
, Toast.LENGTH_SHORT).show() }
            catch (e: ArithmeticException) { Toast.makeText(this, e.message, Toa
st.LENGTH_SHORT).show() }
            catch (e: Throwable) { Toast.makeText(this, "Error", Toast.LENGTH_SH
ORT).show() }
        }

        findViewById<Button>(R.id.btnAdd).setOnClickListener { doCalc('+') }
        findViewById<Button>(R.id.btnSub).setOnClickListener { doCalc('-') }
        findViewById<Button>(R.id.btnMul).setOnClickListener { doCalc('*') }
        findViewById<Button>(R.id.btnDiv).setOnClickListener { doCalc('/') }
    }
}
```

## 13. Risks & Mitigations
- JNI ref leaks → keep calls primitive-only; no refs created.
- Locale decimal separators → rely on Kotlin `toDouble()` which expects dot; con
sider `NumberFormat` for i18n later.
- ABI size → ship only `arm64-v8a` first; add more ABIs if needed.

---
