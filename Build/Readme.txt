***Grove, a programmable toy***

Besides this readme you'll find Grove, an interpreter for a simple scripting language. With this language you can program vector graphics, quite similar to the famous Turtle graphics introduced with Logo.

What makes Grove special is that it couples the concept of imperative programming with Lindenmayer systems. Usually L-Systems are bunch of production rules that operate on strings of symbols, expanding each symbol into larger strings of symbols.

In Grove there are still productions but the symbols are actually sequences of script commands. So the productions can be used to generate executable code. In procedural languages code is structured into series of commands (procedures), too. But these procedures are called explicitly from within itself or other procedures. Not so in grove, where you can create complex sequences of procedure calls by applying simple rules to equally simple axioms.

My aim in developing Grove was to allow the user to explore emergence in an iterative and fun way.

***Getting started***

All you have to do is execute Grove.exe, then drag&drop a scriptfile onto the application window.

Press 'F' to toggle between window and fullscreen mode. 'D' toggles display of the debug log. 'S' toggles display of statistics. 'B' toggles Post-FX.

If you want to modify the scripts open them in your favorite texteditor, modify and save them. The application will detect the change and update accordingly in an instant.

I suggest you look at the examples in the examples folder.

If you want to write your own scripts (and I advice you to give it a try) there's a series of well commented scripts in the tutorial folder. There's also a short script reference in the doc folder.
	
If you have questions feel free to mail me at lithander@gmx.de

Have fun!