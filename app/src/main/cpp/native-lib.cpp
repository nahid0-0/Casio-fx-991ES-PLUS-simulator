#include <jni.h>
#include <stdexcept>
#include <string>
#include "calc.h"
#include "parsing.h"

static void throwJava(JNIEnv* env, const char* clazz, const char* msg) {
    jclass ex = env->FindClass(clazz);
    if (ex != nullptr) {
        env->ThrowNew(ex, msg);
        env->DeleteLocalRef(ex);
    } else {
        // Fallback if class not found
        jclass runtimeEx = env->FindClass("java/lang/RuntimeException");
        if (runtimeEx != nullptr) {
            env->ThrowNew(runtimeEx, "Failed to throw specific exception");
            env->DeleteLocalRef(runtimeEx);
        }
    }
}

extern "C" JNIEXPORT jdouble JNICALL
Java_com_example_calculator_Native_calc(JNIEnv* env, jclass, jdouble a, jchar op, jdouble b) {
    try {
        return static_cast<jdouble>(
            calc(static_cast<double>(a), static_cast<char>(op), static_cast<double>(b))
        );
    } catch (const std::invalid_argument& e) {
        throwJava(env, "java/lang/IllegalArgumentException", e.what());
        return 0.0;
    } catch (const std::runtime_error& e) {
        throwJava(env, "java/lang/ArithmeticException", e.what());
        return 0.0;
    } catch (...) {
        throwJava(env, "java/lang/RuntimeException", "unknown error");
        return 0.0;
    }
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_calculator_Native_parseExpression(JNIEnv* env, jclass, jstring expression) {
    try {
        // Convert jstring to std::string
        const char* nativeString = env->GetStringUTFChars(expression, 0);
        std::string inputExpression(nativeString);
        env->ReleaseStringUTFChars(expression, nativeString);
        
        // Call the parsing function
        std::string result = parseExpression(inputExpression);
        
        // Convert result back to jstring
        return env->NewStringUTF(result.c_str());
    } catch (const std::exception& e) {
        throwJava(env, "java/lang/RuntimeException", e.what());
        return env->NewStringUTF("Error");
    } catch (...) {
        throwJava(env, "java/lang/RuntimeException", "unknown parsing error");
        return env->NewStringUTF("Error");
    }
}

