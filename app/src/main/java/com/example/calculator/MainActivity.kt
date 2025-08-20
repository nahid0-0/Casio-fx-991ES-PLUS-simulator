package com.example.calculator

import android.os.Bundle
import android.util.Log
import android.widget.Button
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity

object Native {
    init {
        System.loadLibrary("calc")
    }
    external fun calc(a: Double, op: Char, b: Double): Double
    external fun parseExpression(expression: String): String
}

class MainActivity : AppCompatActivity() {
    private lateinit var display: TextView
    private lateinit var result: TextView
    private val expression = StringBuilder()
    private var lastResult: String? = null
    private var isNewCalculation = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        
        Log.d("Calculator", "Advanced Calculator with C++ Backend loaded!")

        display = findViewById(R.id.txtDisplay)
        result = findViewById(R.id.txtResult)

        clearAll()
        setupButtons()

        Log.d("Calculator", "Calculator interface initialized successfully!")
        Toast.makeText(this, "Advanced Calculator Ready!", Toast.LENGTH_SHORT).show()
    }

    private fun setupButtons() {
        // Setup digit buttons (0-9)
        val digitButtons = listOf(
            R.id.btn0, R.id.btn1, R.id.btn2, R.id.btn3, R.id.btn4,
            R.id.btn5, R.id.btn6, R.id.btn7, R.id.btn8, R.id.btn9
        )
        digitButtons.forEach { id ->
            findViewById<LinearLayout>(id).setOnClickListener {
                val digitText = when(id) {
                    R.id.btn0 -> "0"
                    R.id.btn1 -> "1"
                    R.id.btn2 -> "2"
                    R.id.btn3 -> "3"
                    R.id.btn4 -> "4"
                    R.id.btn5 -> "5"
                    R.id.btn6 -> "6"
                    R.id.btn7 -> "7"
                    R.id.btn8 -> "8"
                    R.id.btn9 -> "9"
                    else -> ""
                }
                appendToExpression(digitText)
            }
        }

        // Setup decimal point
        findViewById<LinearLayout>(R.id.btnDot).setOnClickListener { 
            appendToExpression(".") 
        }

        // Setup basic operators
        findViewById<LinearLayout>(R.id.btnAdd).setOnClickListener { 
            appendToExpression(" + ") 
        }
        findViewById<LinearLayout>(R.id.btnSubtract).setOnClickListener { 
            appendToExpression(" - ") 
        }
        findViewById<LinearLayout>(R.id.btnMultiply).setOnClickListener { 
            appendToExpression(" * ") 
        }
        findViewById<LinearLayout>(R.id.btnDivide).setOnClickListener { 
            appendToExpression(" / ") 
        }

        // Setup parentheses
        findViewById<LinearLayout>(R.id.btnBracketOpen).setOnClickListener { 
            appendToExpression("(") 
        }
        findViewById<LinearLayout>(R.id.btnBracketClose).setOnClickListener { 
            appendToExpression(")") 
        }

        // Setup advanced functions
        findViewById<LinearLayout>(R.id.btnSqrt).setOnClickListener { 
            appendToExpression("sqrt(") 
        }
        findViewById<LinearLayout>(R.id.btnSquare).setOnClickListener { 
            // If expression is empty or just calculated, use last result
            if (expression.isEmpty() && lastResult != null) {
                expression.append(lastResult)
            }
            appendToExpression("^2") 
        }
        findViewById<LinearLayout>(R.id.btnXPower).setOnClickListener { 
            appendToExpression("^") 
        }
        findViewById<LinearLayout>(R.id.btnPower).setOnClickListener { 
            appendToExpression("inv(") 
        }
        findViewById<LinearLayout>(R.id.btnLog).setOnClickListener { 
            appendToExpression("log(") 
        }
        findViewById<LinearLayout>(R.id.btnLn).setOnClickListener { 
            appendToExpression("ln(") 
        }
        findViewById<LinearLayout>(R.id.btnSin).setOnClickListener { 
            appendToExpression("sin(") 
        }
        findViewById<LinearLayout>(R.id.btnCos).setOnClickListener { 
            appendToExpression("cos(") 
        }

        // Setup special buttons
        findViewById<LinearLayout>(R.id.btnNegative).setOnClickListener { 
            appendToExpression("(-") 
        }

        // Setup scientific notation
        findViewById<LinearLayout>(R.id.btnExp).setOnClickListener { 
            appendToExpression("*10^") 
        }

        // Setup answer button (use last result)
        findViewById<LinearLayout>(R.id.btnAns).setOnClickListener { 
            if (lastResult != null) {
                appendToExpression(lastResult!!)
            }
        }

        // Setup control buttons
        findViewById<LinearLayout>(R.id.btnEquals).setOnClickListener { calculateExpression() }
        findViewById<LinearLayout>(R.id.btnAC).setOnClickListener { clearAll() }
        findViewById<LinearLayout>(R.id.btnDel).setOnClickListener { deleteLast() }

        // Static buttons (not implemented yet)
        setupStaticButtons()
    }

    private fun setupStaticButtons() {
        val staticButtons = listOf(
            R.id.btnShift, R.id.btnAlpha, R.id.btnMode, R.id.btnOn,
            R.id.btnCalc
        )
        
        staticButtons.forEach { id ->
            try {
                when (id) {
                    R.id.btnShift, R.id.btnOn -> {
                        findViewById<Button>(id).setOnClickListener {
                            Toast.makeText(this, "Function not implemented yet", Toast.LENGTH_SHORT).show()
                        }
                    }
                    else -> {
                        findViewById<LinearLayout>(id).setOnClickListener {
                            Toast.makeText(this, "Function not implemented yet", Toast.LENGTH_SHORT).show()
                        }
                    }
                }
            } catch (e: Exception) {
                Log.w("Calculator", "Button not found: $id")
            }
        }
    }

    private fun appendToExpression(text: String) {
        // If we just calculated and user starts typing a new number, clear the expression
        if (isNewCalculation && text.matches(Regex("[0-9.]"))) {
            expression.clear()
            result.text = ""
            isNewCalculation = false
        }
        // If user starts with an operator after a calculation, use the last result
        else if (isNewCalculation && text.trim().matches(Regex("[+\\-*/^]"))) {
            expression.clear()
            if (lastResult != null) {
                expression.append(lastResult)
            }
            result.text = ""
            isNewCalculation = false
        }
        
        expression.append(text)
        updateDisplay()
    }

    private fun calculateExpression() {
        val expressionText = expression.toString().trim()
        
        if (expressionText.isEmpty() || expressionText == "0") {
            return
        }
        
        try {
            Log.d("Calculator", "Sending expression to C++: $expressionText")
            
            // Send the expression to C++ parsing and evaluation system
            val parseResult = Native.parseExpression(expressionText)
            
            Log.d("Calculator", "C++ returned: $parseResult")
            
            // Keep expression visible in display, show result below
            display.text = expressionText  // Keep original expression visible
            result.text = parseResult
            
            // Store result for potential further calculations
            val resultValue = extractResultValue(parseResult)
            if (resultValue != null) {
                lastResult = resultValue
                isNewCalculation = true  // Flag that we just completed a calculation
            }
            
        } catch (e: Exception) {
            val errorMsg = "Error: ${e.message}"
            result.text = errorMsg
            Log.e("Calculator", "Calculation error", e)
        }
    }

    private fun extractResultValue(fullResult: String): String? {
        // Extract the actual result from "Result: 42" format
        return if (fullResult.startsWith("Result: ")) {
            fullResult.substring(8).trim()
        } else if (fullResult.startsWith("Error: ")) {
            null // Don't chain on errors
        } else {
            fullResult.trim() // Fallback
        }
    }

    private fun deleteLast() {
        if (expression.isNotEmpty()) {
            expression.deleteCharAt(expression.length - 1)
            updateDisplay()
        }
    }

    private fun clearAll() {
        expression.clear()
        display.text = "0"
        result.text = ""
        lastResult = null
        isNewCalculation = false
    }

    private fun updateDisplay() {
        display.text = if (expression.isEmpty()) "0" else expression.toString()
    }
}

