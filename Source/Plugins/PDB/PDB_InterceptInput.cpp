// The comments describing the commands are from the python pdb manual:
// http://docs.python.org/3.3/library/pdb.html
// Copyright 1990-2013, Python Software Foundation.

#include "PDB.h"

bool PDB::OnInterceptInput(const wxString &message)
{
	// Remove all white spaces before and after.
	wxString command = message.Strip();

	if (command.IsEmpty())
	{
		// Repeat the last command.
		switch (lastCommand)
		{
		case kNone:
		case kPrint:
			// These can be ignored.
			break;

		case kBreak:
		case kClear:
			expectedOutput = kBreakpoint;
			return true;

		case kWhere:
			expectedOutput = kCallstack;
			return true;

		case kContinue:
		case kNext:
		case kReturn:
		case kStep:
			expectedOutput = kStepping;
			return true;

		case kQuit:
			expectedOutput = kQuitting;
			return true;
		}
	}
	else
	{
		// Try to parse the command and see if we can use the output.
		switch (command[0].GetValue())
		{
		case 'a':
			// a(rgs)
			// Print the argument list of the current function.

			// alias [name [command]]
			// Create an alias called name that executes command. The command
			// must not be enclosed in quotes. Replaceable parameters can be
			// indicated by %1, %2, and so on, while %* is replaced by all the
			// parameters. If no command is given, the current alias for name
			// is shown. If no arguments are given, all aliases are listed.
			//
			// Aliases may be nested and can contain anything that can be
			// legally typed at the pdb prompt.Note that internal pdb commands
			// can be overridden by aliases.Such a command is then hidden until
			// the alias is removed.Aliasing is recursively applied to the
			// first word of the command line; all other words in the line are
			// left alone.
			break;

		case 'b':
			// b(reak) [([filename:]lineno | function) [, condition]]
			// With a lineno argument, set a break there in the current file.
			// With a function argument, set a break at the first executable
			// statement within that function. The line number may be prefixed
			// with a filename and a colon, to specify a breakpoint in another
			// file (probably one that hasn’t been loaded yet). The file is
			// searched on sys.path. Note that each breakpoint is assigned a
			// number to which all the other breakpoint commands refer.
			//
			// If a second argument is present, it is an expression which must
			// evaluate to true before the breakpoint is honored.
			//
			// Without argument, list all breaks, including for each
			// breakpoint, the number of times that breakpoint has been hit,
			// the current ignore count, and the associated condition if any.
			break;

		case 'c':
			// cl(ear) [filename:lineno | bpnumber [bpnumber ...]]
			// With a filename:lineno argument, clear all the breakpoints at
			// this line. With a space separated list of breakpoint numbers,
			// clear those breakpoints. Without argument, clear all breaks
			// (but first ask confirmation).

			// condition bpnumber [condition]
			// Set a new condition for the breakpoint, an expression which must
			// evaluate to true before the breakpoint is honored. If condition
			// is absent, any existing condition is removed; i.e., the
			// breakpoint is made unconditional.

			// command [bpnumber]
			// Specify a list of commands for breakpoint number bpnumber. The
			// commands themselves appear on the following lines. Type a line
			// containing just end to terminate the commands.

			// c(ont(inue))
			// Continue execution, only stop when a breakpoint is encountered.
			if ((command == "c") || (command == "cont") || (command == "continue"))
			{
				lastCommand		= kContinue;
				expectedOutput	= kStepping;
				return true;
			}
			break;

		case 'd':
			// d(own) [count]
			// Move the current frame count (default one) levels down in the
			// stack trace (to a newer frame).

			// disable [bpnumber [bpnumber ...]]
			// Disable the breakpoints given as a space separated list of
			// breakpoint numbers. Disabling a breakpoint means it cannot
			// cause the program to stop execution, but unlike clearing a
			// breakpoint, it remains in the list of breakpoints and can be
			// (re-)enabled.

			// display [expression]
			// Display the value of the expression if it changed, each time
			// execution stops in the current frame.
			//
			// Without expression, list all display expressions for the current
			// frame.
			//
			// New in version 3.2.
			break;

		case 'e':
			// enable [bpnumber [bpnumber ...]]
			// Enable the breakpoints specified.
			break;

		case 'h':
			// h(elp) [command]
			break;

		case 'i':
			// ignore bpnumber [count]
			// Set the ignore count for the given breakpoint number. If count
			// is omitted, the ignore count is set to 0. A breakpoint becomes
			// active when the ignore count is zero. When non-zero, the count
			// is decremented each time the breakpoint is reached and the
			// breakpoint is not disabled and any associated condition
			// evaluates to true.

			// interact
			// Start an interative interpreter (using the code module) whose
			// global namespace contains all the (global and local) names found
			// in the current scope.
			//
			// New in version 3.2.
			break;

		case 'j':
			// j(ump) lineno
			// Set the next line that will be executed. Only available in the
			// bottom-most frame. This lets you jump back and execute code
			// again, or jump forward to skip code that you don’t want to run.
			//
			// It should be noted that not all jumps are allowed – for instance
			// it is not possible to jump into the middle of a for loop or out
			// of a finally clause.
			break;

		case 'l':
			// l(ist) [first[, last]]
			// List source code for the current file. Without arguments, list
			// 11 lines around the current line or continue the previous
			// listing. With . as argument, list 11 lines around the current
			// line. With one argument, list 11 lines around at that line. With
			// two arguments, list the given range; if the second argument is
			// less than the first, it is interpreted as a count.
			//
			// The current line in the current frame is indicated by ->. If an
			// exception is being debugged, the line where the exception was
			// originally raised or propagated is indicated by >> , if it
			// differs from the current line.
			//
			// New in version 3.2: The >> marker.

			// ll | longlist
			// List all source code for the current function or frame. Interesting lines are marked as for list.
			//
			// New in version 3.2.
			break;

		case 'n':
			// n(ext)
			// Continue execution until the next line in the current function
			// is reached or it returns. (The difference between next and step
			// is that step stops inside a called function, while next executes
			// called functions at (nearly) full speed, only stopping at the
			// next line in the current function.)
			if ((command == "n") || (command == "next"))
			{
				lastCommand		= kNext;
				expectedOutput	= kStepping;
				return true;
			}
			break;

		case 'p':
			// p(rint) expression
			// Evaluate the expression in the current context and print its value.

			// pp expression
			// Like the print command, except the value of the expression is
			// pretty-printed using the pprint module.
			break;

		case 'q':
			// q(uit)
			// Quit from the debugger. The program being executed is aborted.
			if ((command == "q") || (command == "quit"))
			{
				lastCommand		= kQuit;
				expectedOutput	= kQuitting;
				return true;
			}
			break;

		case 'r':
			// r(eturn)
			// Continue execution until the current function returns.
			if ((command == "r") || (command == "return"))
			{
				lastCommand		= kReturn;
				expectedOutput	= kStepping;
				return true;
			}

			// run [args ...]
			// restart [args ...]
			// Restart the debugged Python program. If an argument is supplied,
			// it is split with shlex and the result is used as the new
			// sys.argv. History, breakpoints, actions and debugger options are
			// preserved. restart is an alias for run.
			break;

		case 's':
			// s(tep)
			// Execute the current line, stop at the first possible occasion
			// (either in a function that is called or on the next line in the
			// current function).
			if ((command == "s") || (command == "step"))
			{
				lastCommand		= kStep;
				expectedOutput	= kStepping;
				return true;
			}

			// source expression
			// Try to get source code for the given object and display it.
			//
			// New in version 3.2.
			break;

		case 't':
			// tbreak [([filename:]lineno | function) [, condition]]
			// Temporary breakpoint, which is removed automatically when it is
			// first hit. The arguments are the same as for break.
			break;

		case 'u':
			// u(p) [count]
			// Move the current frame count (default one) levels up in the
			// stack trace (to an older frame).

			// unt(il) [lineno]
			// Without argument, continue execution until the line with a
			// number greater than the current one is reached.
			//
			// With a line number, continue execution until a line with a
			// number greater or equal to that is reached.In both cases, also
			// stop when the current frame returns.
			//
			// Changed in version 3.2: Allow giving an explicit line number.

			// undisplay [expression]
			// Do not display the expression any more in the current frame.
			// Without expression, clear all display expressions for the
			// current frame.
			//
			// New in version 3.2.

			// unalias name
			// Delete the specified alias.
			break;

		case 'w':
			// w(here)
			// Print a stack trace, with the most recent frame at the bottom.
			// An arrow indicates the current frame, which determines the
			// context of most commands.
			if ((command == "w") || (command == "where"))
			{
				lastCommand		= kWhere;
				expectedOutput	= kCallstack;
				return true;
			}

			// whatis expression
			// Print the type of the expression.
			break;

		case '!':
			// ! statement
			// Execute the (one-line) statement in the context of the current
			// stack frame. The exclamation point can be omitted unless the
			// first word of the statement resembles a debugger command. To
			// set a global variable, you can prefix the assignment command
			// with a global statement on the same line.
			break;
		}

	}

	// The command wasn't recognized, don't assume anything.
	lastCommand		= kNone;
	expectedOutput	= kUnknown;
	return false;
}
