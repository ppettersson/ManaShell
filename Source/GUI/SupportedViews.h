#ifndef SUPPORTED_VIEWS_H
#define SUPPORTED_VIEWS_H

// Utility struct to keep track on the supported views for the UI in the
// current debugger state.
struct SupportedViews
{
	unsigned	breakpoints	: 1,
				callstack	: 1,
				registers	: 1,
				threads		: 1,
				watch		: 1,
				locals		: 1;

	SupportedViews()
		: breakpoints(false)
		, callstack(false)
		, registers(false)
		, threads(false)
		, watch(false)
		, locals(false)
	{
	}
};

#endif // SUPPORTED_VIEWS_H
