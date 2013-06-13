#ifndef HAVE_json_parser_hpp
#define HAVE_json_parser_hpp
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include "exceptions.hpp"
#include "value.hpp"

namespace json {

	/** Trida ktera umi parsovat JSON */
	class Parser {
		/* Tohle by doxygen mel ignorovat */
		/// @cond
		/** Typ tokenu */
		enum TokenType {
			tSTRING = 256, ///< retezec
			tNUMBER, ///< cislo
			kTRUE, ///< klicove slovo true
			kFALSE, ///< klicove slovo false
			kNULL, ///< klicove slovo null
			tEOF ///< konec vstupu
		};

		/** Lexikalni token */
		struct Token {
			TokenType type; ///< Typ tokenu

			/** Atrubut tokenu */
			struct Attr {
				json::String string; ///< tSTRING
				json::Number number; ///< tNUMBER
			};

			Attr attr; ///< Atribut
		};
		/// @endcond

		std::istream *input; ///< Vstup
		Token token; ///< Posledni token
		size_t line; ///< Cislo radku

		/** Dalsi znak */
		inline int getch();

		/** Vrati znak zpatky do vstupu */
		inline void retch(int ch);
		
		/** Parsuje retezec */
		String lexString();

		/** Parsuje cislo */
		Number lexNumber();

		/** Preskoci mezery a komentare */
		void skipSpaces();

		/** Ziska dalsi token a do ho do promenne token */
		void lex();

		/** Ziska hodnotu z tokenu */
		Value parseValue();

		/** Prevede typ tokenu na jmeno */
		std::string tokenType2Name(TokenType type);

		/** Vyhodi ParseError s informacemi o radku */
		void parseError(const std::string &msg);

		/** Vyhodi SyntaxError s informacemi o radku */
		void syntaxError(const std::string &msg);

		/** Zkontroluje token jesli ma typ type, jinak vyhodi SyntaxError */
		void expectToken(TokenType type); 
	public:
		/** Parsuje JSON ze vstupu.
		 * Zpracuje JSON ze vstupu a vrati vysledek.
		 *
		 * @param in Vstupni proud ze ktereho se ctou data
		 * @throw ParseError Chyba pri parsovani
		 * @throw SyntaxError Chyba syntaxe
		 */
		Value parse(std::istream &in);
	};
}
#endif
