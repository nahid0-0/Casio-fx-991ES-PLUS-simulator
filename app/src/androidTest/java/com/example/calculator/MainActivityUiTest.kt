package com.example.calculator

import androidx.test.core.app.ActivityScenario
import androidx.test.espresso.Espresso.onView
import androidx.test.espresso.action.ViewActions.click
import androidx.test.espresso.action.ViewActions.replaceText
import androidx.test.espresso.assertion.ViewAssertions.matches
import androidx.test.espresso.matcher.ViewMatchers.withId
import androidx.test.espresso.matcher.ViewMatchers.withText
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class MainActivityUiTest {

    @Test
    fun addButtonDisplaysSum() {
        ActivityScenario.launch(MainActivity::class.java)

        onView(withId(R.id.inputA)).perform(replaceText("3"))
        onView(withId(R.id.inputB)).perform(replaceText("4"))
        onView(withId(R.id.btnAdd)).perform(click())
        onView(withId(R.id.txtResult)).check(matches(withText("7.0")))
    }

    @Test
    fun divideByZeroShowsError() {
        ActivityScenario.launch(MainActivity::class.java)

        onView(withId(R.id.inputA)).perform(replaceText("5"))
        onView(withId(R.id.inputB)).perform(replaceText("0"))
        onView(withId(R.id.btnDiv)).perform(click())
        // Result text should remain unchanged (empty) due to error
        onView(withId(R.id.txtResult)).check(matches(withText("")))
    }
}

