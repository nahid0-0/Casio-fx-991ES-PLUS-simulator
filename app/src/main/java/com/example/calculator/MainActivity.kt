package com.example.calculator

import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.Button
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity

object Native {
    private var isLibraryLoaded = false
    
    init {
        try {
            System.loadLibrary("calc")
            isLibraryLoaded = true
            Log.d("Calculator", "Native library loaded successfully")
        } catch (e: Exception) {
            Log.e("Calculator", "Failed to load native library: ${e.message}")
            isLibraryLoaded = false
        }
    }
    
    external fun calc(a: Double, op: Char, b: Double): Double
    external fun parseExpression(expression: String): String
    
    fun isAvailable(): Boolean = isLibraryLoaded
}

class MainActivity : AppCompatActivity() {
    private lateinit var display: TextView
    private lateinit var result: TextView
    private val expression = StringBuilder()
    private var lastResult: String? = null
    private var isNewCalculation = false

    override fun onCreate(savedInstanceState: Bundle?) {
        try {
            super.onCreate(savedInstanceState)
            Log.d("Calculator", "Starting calculator initialization...")
            
            setContentView(R.layout.activity_main)
            Log.d("Calculator", "Layout loaded successfully")

            display = findViewById(R.id.txtDisplay)
            result = findViewById(R.id.txtResult)
            Log.d("Calculator", "Display elements found")

            clearAll()
            Log.d("Calculator", "Display cleared")
            
            // Setup all calculator buttons
            setupButtons()
            Log.d("Calculator", "Buttons setup complete")

            val statusMessage = if (Native.isAvailable()) {
                "✅ Calculator Ready!\nC++ Engine: Loaded"
            } else {
                "⚠️ Calculator Ready!\nC++ Engine: Failed to Load (UI Mode Only)"
            }
            
            Toast.makeText(this, statusMessage, Toast.LENGTH_LONG).show()
            Log.d("Calculator", "Calculator initialization complete: ${statusMessage}")
            
        } catch (e: Exception) {
            Log.e("Calculator", "FATAL ERROR during initialization", e)
            Toast.makeText(this, "Calculator failed to start: ${e.message}", Toast.LENGTH_LONG).show()
            // Don't crash, just show error
        }
    }

    private fun setupButtons() {
        // Setup digit buttons (0-9)
        val digitButtons = listOf(
            R.id.btn0, R.id.btn1, R.id.btn2, R.id.btn3, R.id.btn4,
            R.id.btn5, R.id.btn6, R.id.btn7, R.id.btn8, R.id.btn9
        )
        digitButtons.forEach { id ->
            findViewById<View>(id)?.setOnClickListener {
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
        findViewById<View>(R.id.btnDot)?.setOnClickListener { 
            appendToExpression(".") 
        }

        // Setup basic operators
        findViewById<View>(R.id.btnAdd)?.setOnClickListener { 
            appendToExpression(" + ") 
        }
        findViewById<View>(R.id.btnSubtract)?.setOnClickListener { 
            appendToExpression(" - ") 
        }
        findViewById<View>(R.id.btnMultiply)?.setOnClickListener { 
            appendToExpression(" * ") 
        }
        findViewById<View>(R.id.btnDivide)?.setOnClickListener { 
            appendToExpression(" / ") 
        }

        // Setup parentheses
        findViewById<View>(R.id.btnParenOpen)?.setOnClickListener { 
            appendToExpression("(") 
        }
        findViewById<View>(R.id.btnParenClose)?.setOnClickListener { 
            appendToExpression(")") 
        }

        // Setup advanced functions
        findViewById<View>(R.id.btnSqrt)?.setOnClickListener { 
            appendToExpression("sqrt(") 
        }
        findViewById<View>(R.id.btnSquare)?.setOnClickListener { 
            // If expression is empty or just calculated, use last result
            if (expression.isEmpty() && lastResult != null) {
                expression.append(lastResult)
            }
            appendToExpression("^2") 
        }
        findViewById<View>(R.id.btnPower)?.setOnClickListener { 
            appendToExpression("inv(") 
        }
        findViewById<View>(R.id.btnPowerXY)?.setOnClickListener { 
            appendToExpression("^") 
        }
        findViewById<View>(R.id.btnLog)?.setOnClickListener { 
            appendToExpression("log(") 
        }
        findViewById<View>(R.id.btnLn)?.setOnClickListener { 
            appendToExpression("ln(") 
        }
        findViewById<View>(R.id.btnSin)?.setOnClickListener { 
            appendToExpression("sin(") 
        }
        findViewById<View>(R.id.btnCos)?.setOnClickListener { 
            appendToExpression("cos(") 
        }
        findViewById<View>(R.id.btnCube)?.setOnClickListener { 
            // If expression is empty or just calculated, use last result
            if (expression.isEmpty() && lastResult != null) {
                expression.append(lastResult)
            }
            appendToExpression("^3") 
        }
        findViewById<View>(R.id.btnAbs)?.setOnClickListener { 
            appendToExpression("abs(") 
        }

        // Setup special buttons
        findViewById<View>(R.id.btnNegate)?.setOnClickListener { 
            appendToExpression("(-") 
        }

        // Setup scientific notation
        findViewById<View>(R.id.btnExp)?.setOnClickListener { 
            appendToExpression("*10^") 
        }

        // Setup answer button (use last result)
        findViewById<View>(R.id.btnAns)?.setOnClickListener { 
            if (lastResult != null) {
                appendToExpression(lastResult!!)
            }
        }

        // Setup control buttons
        findViewById<View>(R.id.btnEquals)?.setOnClickListener { calculateExpression() }
        findViewById<View>(R.id.btnAC)?.setOnClickListener { clearAll() }
        findViewById<View>(R.id.btnDel)?.setOnClickListener { deleteLast() }

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
                        findViewById<Button>(id)?.setOnClickListener {
                            Toast.makeText(this, "Function not implemented yet", Toast.LENGTH_SHORT).show()
                        }
                    }
                    else -> {
                        findViewById<View>(id)?.setOnClickListener {
                            Toast.makeText(this, "Function not implemented yet", Toast.LENGTH_SHORT).show()
                        }
                    }
                }
            } catch (e: Exception) {
                Log.w("Calculator", "Button not found: $id")
            }
        }
    }

    // HELPER FUNCTIONS - These work with just the display elements
    fun appendToExpression(text: String) {
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

    fun calculateExpression() {
        val expressionText = expression.toString().trim()
        
        if (expressionText.isEmpty() || expressionText == "0") {
            return
        }
        
        try {
            Log.d("Calculator", "Attempting to evaluate: $expressionText")
            
            if (!Native.isAvailable()) {
                result.text = "Native library not available - UI only mode"
                Log.w("Calculator", "Native library not loaded, cannot evaluate expressions")
                return
            }
            
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

    fun deleteLast() {
        if (expression.isNotEmpty()) {
            expression.deleteCharAt(expression.length - 1)
            updateDisplay()
        }
    }

    fun clearAll() {
        expression.clear()
        display.text = "0"
        result.text = ""
        lastResult = null
        isNewCalculation = false
    }

    private fun updateDisplay() {
        display.text = if (expression.isEmpty()) "0" else expression.toString()
    }

    // PUBLIC METHODS FOR YOUR NEW UI TO USE:
    // - appendToExpression(text: String) - Add text to calculator
    // - calculateExpression() - Evaluate the current expression  
    // - clearAll() - Reset everything
    // - deleteLast() - Remove last character
}
