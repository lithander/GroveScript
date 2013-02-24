This is an early preview of the still nameless LSystem-Toy I'm developing.

To see it in action execute Grove.exe, then drag&drop the example scripts onto the application window.

If you want to modify the scripts open them in your favorite texteditor, modify and save them. The application will detect the change and update accordingly.

***Introduction to the scripting language***

The scripting language is meant to be easy to pick up and modify. There are a handful of operations (one per line) that depending on their type take zero, one ore more (comma-sepearted) parameters.

A #-character followed by a name will mark the start of a sequence of commands that can be referenced by the given name. You can execute a sequence with the 'Run' command.

The most powerful command is 'Grow' which will take a string of Sequences (Axiom) as first parameter and the number of iterations as 2nd parameter. It applies prudction rules to the string making it longer and more complex and finally executes all command sequences in the resulting string.

Only very simple productions are supported for now and look like this:

A B => A B C

The string before the '=>' is the pattern to look for. Once found the production rule will replace the pattern with the string after the '=>'.

So, with the above production rule the command "grow A B, 10" would execute A, B once and then 10 times C.

There are other commands and some macro's that will unfold to commands such as 'while', 'repeat' etc

Here's a list of all commands supported. A more thorough documentation will follow eventually!

***OPERATIONS***
Note: All number parameters can be given as algebraic expressions!

Move x (number)
	Will move the turtle by the given length
Rotate x (number)
	Will rotate the tutle by the given angle
Size x (number) 
	Will set the width of the line the turtle draws
Rgb red, green, blue (3 numbers in the range 0 to 1)
	Will set the color of the turtle 
Hsv hue, saturation, value (3 numbers in the range 0 to 1)
	Will set the color of the turtle
Pos x, y (numbers)
	Will set the absolute position of the turtle in pixel
Dir x (number)
	Will set the absolute rotation of the turtle in pixel
Set name, value (string, number)
	Will set a variables value. Variables can be used in expressions.
Push
	Pushes the turtle state and all variables to a stack
Pop
	Restores the turtle state and all variables values to the top of the stack
Out output (string or expression)
	Generates a debug output of the given string or expression. Strings need to be prefixed by a ' character.
Run name (string)
	Executes the sequence of commands with the given name.
Grow axiom, iterations (string and value)
	Grows and finally executes the axiom according to the production rules

***MACROS***

Macros will unfold to operations but they are easier to work with then expressing the same thing directly with operations would be. Especially as I haven't documented the operations control-flow macros use! :)
Macros operate on blocks of code. Blocks are formed by indention using Tab characters.

Raise name (string)
	Raises a variables value just like 'Set name, name + 1' would.
Lower name (string)
	Lowers a variable by one. The same as 'Set name, name - 1' would.
Repeat i (number)
	Repeat the following block of code i times
While cnd (logical expression)
	Repeat the following block of code while cnd evaluates to true
Until cnd (logical expression)
	Repeat the following block of code while cnd evaluates to false
If cnd (logical expressions)
	Execute the following block only if cnd is true
Else
	Only usable after an If macro. Executes the following block if previous If-condition was false. Can be chained with further If macros.

Here's an example of flow control macros to illustrate how they work together:

#letscount
set i, 0
repeat 2
	while i < 12
		raise i
		if i%3=0
			out i, 'multiple of 3
		else if i%2=1
			out i, 'uneven
		else
			out i
	until i <= 0
		lower i, 2
		out i

***Expressions***

Algebraic Expressions adhere to the usual rules and support the following functions:
	sin, cos, tan, asin, acos, atan, exp, ln, floor, ceil, abs, time, min, max, clamp and frac.

Logical Expressions use the following keywords as operators/literals.
	not, and, or, true, false, is, isnt
	
If you have questions feel free to mail me at lithander@gmx.de

Have fun!