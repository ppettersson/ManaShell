#include "SourceEditor.h"
#include "wx/filename.h"
#include "MainFrame.h"

SourceEditor::SourceEditor(MainFrame *parent)
	: wxStyledTextCtrl(parent)
	, host(parent)
{
	// In debug mode, always let the source files be read-only.
	SetReadOnly(true);

	// Default to tabs that are four spaces and insert actual tabs.
	SetTabWidth(4);
	SetUseTabs(true);

	// Default to showing max line length.
	SetEdgeColumn(80);
	SetEdgeMode(wxSTC_EDGE_LINE);

	// Don't wrap by default.
	SetWrapMode(wxSTC_WRAP_NONE);

	// Set up a regular monospace font.
	wxFont font(10, wxMODERN, wxNORMAL, wxNORMAL);
	StyleSetFont(wxSTC_STYLE_DEFAULT, font);

	//SetProperty("Caret.HighlightCurrentLine", "1");

	SetupMargins();

	MarkerDefine(kBreakpoint, wxSTC_MARK_CIRCLE, wxNullColour, wxColour(255, 0, 0));
	MarkerDefine(kDebuggerNextLine, wxSTC_MARK_SHORTARROW, wxNullColour, wxColour(255, 255, 0));

	// Try to keep the current line centered.
	SetYCaretPolicy(wxSTC_CARET_STRICT | wxSTC_CARET_EVEN, 0);
}

SourceEditor::~SourceEditor()
{
}

bool SourceEditor::Load(const wxString &fileName, unsigned line, bool moveDebugMarker)
{
	// Only reload the file if necessary.
	if (!fileName.IsEmpty() && (fileName != currentFile))
	{
		// Remove all markers.
		MarkerDeleteAll(kDebuggerNextLine);
		MarkerDeleteAll(kBreakpoint);

		// Scintilla won't allow any modifications at all to the buffer if
		// we're in read only mode.
		SetReadOnly(false);
		if (!OpenFile(fileName))
			return false;
		SetReadOnly(true);

		// Update the syntax highlighting based on the file extension.
		currentFile = fileName;
		SetupHighlighting();
	}

	// Scintilla is 1 based, not 0 based.
	if (line > 0)
	{
		GotoLine(line - 1);

		if (moveDebugMarker)
		{
			MarkerDeleteAll(kDebuggerNextLine);
			MarkerAdd(line - 1, kDebuggerNextLine);
		}
	}
	return true;
}

void SourceEditor::StopDebugging()
{
	MarkerDeleteAll(kDebuggerNextLine);

	SetReadOnly(false);
	ClearAll();
	SetReadOnly(true);

	workingDir	= "";
	currentFile	= "";
	sourceMapping.clear();
}

void SourceEditor::DisableDebugMarker()
{
	MarkerDeleteAll(kDebuggerNextLine);
}

void SourceEditor::AddBreakpoint(unsigned line)
{
	MarkerAdd(line - 1, kBreakpoint);
}

void SourceEditor::RemoveBreakpoint(unsigned line)
{
	MarkerDelete(line - 1, kBreakpoint);
}

void SourceEditor::RemoveAllBreakpoints()
{
	MarkerDeleteAll(kBreakpoint);
}

void SourceEditor::SetupMargins()
{
	// There are three line margins:
	//	[>]		Breakpoint marker and/or the next line to be run when stepping.
	//	[####]	Line number, this can be toggled.
	//	[-]		Block folding, this can be toggled.

	// Breakpoint marker
	SetMarginWidth(kMarker, 16);	// ToDo!
	SetMarginType(kMarker, wxSTC_MARGIN_SYMBOL);
	//SetMarginMask()

	// Line number.
	int width = TextWidth(wxSTC_STYLE_LINENUMBER, wxT("_9999"));
	SetMarginWidth(kLineNumber, width);
	StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(75, 75, 75));
	StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(220, 220, 220));
	SetMarginType(kLineNumber, wxSTC_MARGIN_NUMBER);

	// Block folding.
	//SetMarginType(kBlockFolding, wxSTC_MARGIN_SYMBOL);
	//SetMarginWidth(kBlockFolding, 15);
	//SetMarginMask(kBlockFolding, wxSTC_MASK_FOLDERS);
	//StyleSetBackground(kBlockFolding, wxColor(200, 200, 200));
	//SetMarginSensitive(kBlockFolding, true);

	//SetProperty("fold", "1");
	//SetProperty("fold.comment", "1");
	//SetProperty("fold.compact", "1");

	//wxColor grey(100, 100, 100);
	//MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_ARROW);
	//MarkerSetForeground(wxSTC_MARKNUM_FOLDER, grey);
	//MarkerSetBackground(wxSTC_MARKNUM_FOLDER, grey);

	//MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_ARROWDOWN);
	//MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, grey);
	//MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, grey);

	//MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_EMPTY);
	//MarkerSetForeground(wxSTC_MARKNUM_FOLDERSUB, grey);
	//MarkerSetBackground(wxSTC_MARKNUM_FOLDERSUB, grey);

	//MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_ARROW);
	//MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, grey);
	//MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, "WHITE");

	//MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_ARROWDOWN);
	//MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, grey);
	//MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, "WHITE");

	//MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_EMPTY);
	//MarkerSetForeground(wxSTC_MARKNUM_FOLDERMIDTAIL, grey);
	//MarkerSetBackground(wxSTC_MARKNUM_FOLDERMIDTAIL, grey);

	//MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_EMPTY);
	//MarkerSetForeground(wxSTC_MARKNUM_FOLDERTAIL, grey);
	//MarkerSetBackground(wxSTC_MARKNUM_FOLDERTAIL, grey);

	// Set up so we get events when clicking the margin.
	SetMarginSensitive(kMarker, true);
	SetMarginSensitive(kLineNumber, true);
	Bind(wxEVT_STC_MARGINCLICK, &SourceEditor::OnMarginClick, this);

	// Unused.
	SetMarginWidth(kUnused0, 0);
	SetMarginWidth(kUnused1, 0);
}

void SourceEditor::SetupHighlighting()
{
	StyleClearAll();

	// ToDo: This should really be read from some kind of configuration file.
	// Ideally it would something standardish like '*.tmTheme'.

	const char *cppPatterns[] =
	{
		"c", "cc", "cpp", "cxx", "c++",
		"h", "hh", "hpp", "hxx", "h++"
	};

	const char *pythonPatterns[] =
	{
		"py"
	};

	const char *javaPatterns[] =
	{
		"java"
	};

	wxString ext = wxFileName(currentFile).GetExt().MakeLower();

	enum Syntax
	{
		kInvalid,
		kCpp,
		kPython,
		kJava
	} syntax = kInvalid;

	if (1)
	{
		for (int i = 0; i < WXSIZEOF(cppPatterns); ++i)
			if (ext == cppPatterns[i])
			{
				syntax = kCpp;
				break;
			}
	}
	if (syntax == kInvalid)
	{
		for (int i = 0; i < WXSIZEOF(pythonPatterns); ++i)
			if (ext == pythonPatterns[i])
			{
				syntax = kPython;
				break;
			}
	}
	if (syntax == kInvalid)
	{
		for (int i = 0; i < WXSIZEOF(javaPatterns); ++i)
			if (ext == javaPatterns[i])
			{
				syntax = kJava;
				break;
			}
	}

	switch (syntax)
	{
	case kCpp:		SetupCpp();		break;
	case kPython:	SetupPython();	break;
	case kJava:		SetupJava();	break;
	}
}

void SourceEditor::SetupCpp()
{
	SetLexer(wxSTC_LEX_CPP);

	//StyleSetForeground(wxSTC_C_DEFAULT, wxColour(0, 0, 0));

	wxColour comment(0, 128, 0);
	StyleSetForeground(wxSTC_C_COMMENT, comment);
	StyleSetForeground(wxSTC_C_COMMENTLINE, comment);
	//StyleSetForeground(wxSTC_C_COMMENTDOC, comment);
	//StyleSetForeground(wxSTC_C_COMMENTLINEDOC, comment);
	//StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORD, comment);
	//StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORDERROR, comment);
	//StyleSetForeground(wxSTC_C_PREPROCESSORCOMMENT, comment);

	StyleSetForeground(wxSTC_C_WORD, wxColour(0, 0, 255));
	StyleSetForeground(wxSTC_C_WORD2, wxColour(0, 0, 196));

	StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(0, 0, 255));
	StyleSetForeground(wxSTC_C_OPERATOR, wxColour(0, 0, 150));
	StyleSetForeground(wxSTC_C_IDENTIFIER, wxColour(0, 0, 32));

	wxColour num(150, 0, 150);
	wxColour str(150, 0, 0);
	StyleSetForeground(wxSTC_C_NUMBER, num);
	StyleSetForeground(wxSTC_C_STRING, str);
	StyleSetForeground(wxSTC_C_STRINGRAW, str);
	StyleSetForeground(wxSTC_C_HASHQUOTEDSTRING, str);
	StyleSetForeground(wxSTC_C_CHARACTER, num);
	//StyleSetForeground(wxSTC_C_UUID, wxColour(0, 0, 0));

	//StyleSetForeground(wxSTC_C_STRINGEOL, wxColour(0, 0, 0));
	//StyleSetForeground(wxSTC_C_VERBATIM, wxColour(0, 0, 0));
	//StyleSetForeground(wxSTC_C_REGEX, wxColour(0, 0, 0));
	//StyleSetForeground(wxSTC_C_GLOBALCLASS, wxColour(0, 0, 0));
	//StyleSetForeground(wxSTC_C_TRIPLEVERBATIM, wxColour(0, 0, 0));

	StyleSetItalic(wxSTC_C_COMMENT, true);
	StyleSetItalic(wxSTC_C_COMMENTLINE, true);
	StyleSetItalic(wxSTC_C_COMMENTDOC, true);
	StyleSetItalic(wxSTC_C_COMMENT, true);

	StyleSetBold(wxSTC_C_WORD, true);
	StyleSetBold(wxSTC_C_WORD2, true);
	StyleSetBold(wxSTC_C_OPERATOR, true);

	SetKeyWords(0,	"alignas alignof and and_eq asm auto bitand bitor bool "
					"break case catch char char16_t char32_t class compl "
					"const constexpr const_cast continue decltype default "
					"delete do double dynamic_cast else enum explicit export "
					"extern false float for friend goto if inline int long "
					"mutable namespace new noexcept not not_eq nullptr "
					"operator or or_eq private protected public register "
					"reintpret_cast return short signed static static_assert "
					"static_cast struct switch template this thread_local "
					"throw true try typedef typeid typename union unsigned "
					"using virtual void volatile wchar_t while xor xor_eq");
	SetKeyWords(1,	"override final");
}

void SourceEditor::SetupPython()
{
	SetLexer(wxSTC_LEX_PYTHON);

	//StyleSetForeground(wxSTC_P_DEFAULT, wxColour(0, 0, 0));

	wxColour comment(0, 128, 0);
	StyleSetForeground(wxSTC_P_COMMENTLINE, comment);
	StyleSetForeground(wxSTC_P_COMMENTBLOCK, comment);

	StyleSetForeground(wxSTC_P_WORD, wxColour(0, 0, 255));

	StyleSetForeground(wxSTC_P_CLASSNAME, wxColour(32, 0, 0));
	StyleSetForeground(wxSTC_P_DEFNAME, wxColour(0, 32, 0));
	StyleSetForeground(wxSTC_P_OPERATOR, wxColour(0, 0, 150));
	StyleSetForeground(wxSTC_P_IDENTIFIER, wxColour(0, 0, 32));
	StyleSetForeground(wxSTC_P_DECORATOR, wxColour(0, 0, 64));

	wxColour num(150, 0, 150);
	wxColour str(150, 0, 0);
	StyleSetForeground(wxSTC_P_NUMBER, num);
	StyleSetForeground(wxSTC_P_STRING, str);
	//StyleSetForeground(wxSTC_P_STRINGEOL, str);
	StyleSetForeground(wxSTC_P_CHARACTER, num);
	StyleSetForeground(wxSTC_P_TRIPLE, num);
	StyleSetForeground(wxSTC_P_TRIPLEDOUBLE, num);

	StyleSetItalic(wxSTC_P_COMMENTLINE, true);
	StyleSetItalic(wxSTC_P_COMMENTBLOCK, true);

	StyleSetBold(wxSTC_P_WORD, true);
	StyleSetBold(wxSTC_P_WORD2, true);
	StyleSetBold(wxSTC_P_OPERATOR, true);

	SetKeyWords(0,	"and as assert break class continue def del elif else "
					"except exec finally for from global if import in is "
					"lambda not or pass print raise return try while with "
					"yield");
}

void SourceEditor::SetupJava()
{
	// Java is using the Cpp lexer.
	SetLexer(wxSTC_LEX_CPP);

	//StyleSetForeground(wxSTC_C_DEFAULT, wxColour(0, 0, 0));

	wxColour comment(0, 128, 0);
	StyleSetForeground(wxSTC_C_COMMENT, comment);
	StyleSetForeground(wxSTC_C_COMMENTLINE, comment);
	StyleSetForeground(wxSTC_C_COMMENTDOC, comment);
	StyleSetForeground(wxSTC_C_COMMENTLINEDOC, comment);
	StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORD, comment);
	StyleSetForeground(wxSTC_C_COMMENTDOCKEYWORDERROR, comment);
	StyleSetForeground(wxSTC_C_PREPROCESSORCOMMENT, comment);

	StyleSetForeground(wxSTC_C_WORD, wxColour(0, 0, 255));
	StyleSetForeground(wxSTC_C_WORD2, wxColour(150, 0, 150));

	StyleSetForeground(wxSTC_C_PREPROCESSOR, wxColour(0, 0, 255));
	StyleSetForeground(wxSTC_C_OPERATOR, wxColour(0, 0, 150));
	StyleSetForeground(wxSTC_C_IDENTIFIER, wxColour(0, 0, 32));

	wxColour num(150, 0, 150);
	wxColour str(150, 0, 0);
	StyleSetForeground(wxSTC_C_NUMBER, num);
	StyleSetForeground(wxSTC_C_STRING, str);
	StyleSetForeground(wxSTC_C_STRINGRAW, str);
	StyleSetForeground(wxSTC_C_HASHQUOTEDSTRING, str);
	StyleSetForeground(wxSTC_C_CHARACTER, num);
	//StyleSetForeground(wxSTC_C_UUID, wxColour(0, 0, 0));

	//StyleSetForeground(wxSTC_C_STRINGEOL, wxColour(0, 0, 0));
	//StyleSetForeground(wxSTC_C_VERBATIM, wxColour(0, 0, 0));
	//StyleSetForeground(wxSTC_C_REGEX, wxColour(0, 0, 0));
	//StyleSetForeground(wxSTC_C_GLOBALCLASS, wxColour(0, 0, 0));
	//StyleSetForeground(wxSTC_C_TRIPLEVERBATIM, wxColour(0, 0, 0));

	StyleSetItalic(wxSTC_C_COMMENT, true);
	StyleSetItalic(wxSTC_C_COMMENTLINE, true);
	StyleSetItalic(wxSTC_C_COMMENTDOC, true);
	StyleSetItalic(wxSTC_C_COMMENT, true);

	StyleSetBold(wxSTC_C_WORD, true);
	StyleSetBold(wxSTC_C_WORD2, true);
	StyleSetBold(wxSTC_C_OPERATOR, true);

	SetKeyWords(0,	"abstract assert boolean break byte case catch char class "
					"const continue default do double else enum extends final "
					"finally float for goto if implements import instanceof "
					"int interface long native new package private protected "
					"public return short static strictfp super switch "
					"synchronized this throw throws transient try void "
					"volatile while");
	SetKeyWords(1,	"false null true");
}

bool SourceEditor::OpenFile(const wxString &fileName)
{
	// First try to open it as is.
	if (wxFileExists(fileName) && LoadFile(fileName))
		return true;

	// If it was a relative path, then try to figure out the absolute
	// path from the working directory.
	wxFileName fn;
	fn.Assign(fileName);
	if (fn.IsRelative())
	{
		if (fn.MakeAbsolute(workingDir))
		{
			wxString fullPath = fn.GetFullPath();
			if (wxFileExists(fullPath) && LoadFile(fullPath))
				return true;
		}
	}

	// Check if we already have this file mapped.
	std::map<wxString, wxString>::iterator i = sourceMapping.find(fileName);
	if (i != sourceMapping.end())
	{
		const wxString &fullPath = i->second;
		if (wxFileExists(fullPath) && LoadFile(fullPath))
			return true;
	}

	// If that fails as well, then ask the user to locate the file.
	wxString result = wxFileSelector(wxString::Format("Please locate this file: %s", fileName),
									 fn.GetPath(), fn.GetName(), fn.GetExt(),
									 "All files (*.*)|*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	// If the dialog was cancelled then this is empty.
	if (!result.IsEmpty())
	{
		if (wxFileExists(result) && LoadFile(result))
		{
			// Store this mapping in case it's requested again.
			sourceMapping[fileName] = result;
			return true;
		}
	}

	wxMessageBox(wxString::Format("Failed to open file: %s", fileName), "Error", wxOK | wxCENTRE | wxICON_ERROR, GetParent());
	return false;
}

void SourceEditor::OnMarginClick(wxStyledTextEvent &event)
{
	if (event.GetMargin() == kBlockFolding)
	{
		int	lineClick	= LineFromPosition(event.GetPosition()),
			levelClick	= GetFoldLevel(lineClick);

		if ((levelClick & wxSTC_FOLDLEVELHEADERFLAG) > 0)
			ToggleFold(lineClick);
	}
	else
	{
		int	lineClick	= LineFromPosition(event.GetPosition());
		host->RequestBreakpoint(currentFile, lineClick + 1);
	}
}
