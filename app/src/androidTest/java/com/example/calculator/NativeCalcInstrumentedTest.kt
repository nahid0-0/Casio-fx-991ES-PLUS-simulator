package com.example.calculator

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.assertEquals
import org.junit.Assert.fail
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class NativeCalcInstrumentedTest {

    @Test
    fun testAddition() {
        assertEquals(7.0, Native.calc(3.0, '+', 4.0), 0.0)
    }

    @Test
    fun testSubtraction() {
        assertEquals(-1.0, Native.calc(3.0, '-', 4.0), 0.0)
    }

    @Test
    fun testMultiplication() {
        assertEquals(12.0, Native.calc(3.0, '*', 4.0), 0.0)
    }

    @Test
    fun testDivision() {
        assertEquals(2.0, Native.calc(8.0, '/', 4.0), 0.0)
    }

    @Test
    fun testDivisionByZeroThrows() {
        try {
            Native.calc(1.0, '/', 0.0)
            fail("Expected ArithmeticException")
        } catch (e: ArithmeticException) {
            // expected
        }
    }

    @Test
    fun testInvalidOperatorThrows() {
        try {
            Native.calc(1.0, '#', 2.0)
            fail("Expected IllegalArgumentException")
        } catch (e: IllegalArgumentException) {
            // expected
        }
    }
}

