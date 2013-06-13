#ifndef HAVE_json_exceptions_hpp
#define HAVE_json_exceptions_hpp
#include <string>

namespace json {
	/** Vyjimka */
	class Exception {
	protected:
		std::string mMessage; ///< Zprava
	public:
		/** Zprava je prazdna */
		Exception() { }

		/** Nastavi zpravu */
		explicit Exception(std::string msg): mMessage(msg) { };

		/** Ziska zpravu */
		std::string getMessage() { return mMessage; }

		/** Ziska zpravu */
		std::string what() { return mMessage; }
	};

	/** Typova chyba */
	class TypeError: public Exception {
	public:
		/** Nastavi zpravu */
		TypeError(std::string msg) { mMessage = msg; }
	};

	/** Chyba pri parsovani */
	class ParseError: public Exception {
	public:
		/** Nastavi zpravu */
		ParseError(std::string msg) { mMessage = msg; }
	};

	/** Syntakticka chyba */
	class SyntaxError: public Exception {
	public:
		/** Nastavi zpravu */
		SyntaxError(std::string msg) { mMessage = msg; }
	};

	/** Chyba v datech */
	class DataError: public Exception {
	public:
		/** Nastavi zpravu */
		DataError(std::string msg) { mMessage = msg; }
	};

}
#endif
