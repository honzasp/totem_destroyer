#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include "parser.hpp"
#include "value.hpp"

namespace json {
	std::string Parser::tokenType2Name(TokenType type)
	{
		switch(type) {
			case tSTRING: 
				return "string";
			case tNUMBER:
				return "number";
			case kTRUE:
				return "'true'";
			case kFALSE:
				return "'false'";
			case kNULL:
				return "'null'";
			case tEOF:
				return "end of file";
			default:
				return std::string("'") + (char)type + std::string("'");
		}
	}

	inline int Parser::getch()
	{
		int ch = input->get();
		if(ch == '\n')
			line++;
		return ch;
	}

	inline void Parser::retch(int ch)
	{
		if(ch == '\n')
			line--;
		input->putback(ch);
	}

	void Parser::parseError(const std::string &msg)
	{
		std::stringstream str;
		str << "Line " << line << ": " << msg;
		throw ParseError(str.str());
	}

	void Parser::syntaxError(const std::string &msg)
	{
		std::stringstream str;
		str << "Line " << line << ": " << msg;
		throw SyntaxError(str.str());
	}

	void Parser::expectToken(TokenType type)
	{
		if(token.type != type)
			syntaxError("Expected " + tokenType2Name(type) + ", no " + tokenType2Name(token.type));
	}

	void Parser::skipSpaces()
	{
		while(*input) {
			int ch;
			ch = getch();
			if(std::isspace(ch))
				; // preskocime mezery
			else if(ch == '/') { // mozna zacatek komentare // nebo /*
				int nextch = getch();
				if(nextch == '/') { // komentar //
					while(*input && getch() != '\n') // preskocime zbytek radku
						;
				} else if(nextch == '*') { // komentar /* */
					while(*input) {
						ch = getch();
						if(ch == '*')  {
							if((nextch = getch()) == '/')
								break;
							else 
								retch(nextch);
						}
					}
				} else /* neco uplne jineho */
					retch(nextch);
			} else {
				retch(ch);
				break;
			}
		}
	}

	String Parser::lexString()
	{
		String str;
		int ch;
		while(*input) {
			ch = getch();
			if(ch == '"')
				break;
			else if(ch == '\n') {
				line++;
				str.push_back(ch);
			} else if(ch == '\\') {
				ch = getch();
				switch(ch) {
					case 'b':
						str.push_back('\b');
						break;
					case 'f':
						str.push_back('\f');
						break;
					case 'n':
						str.push_back('\n');
						break;
					case 'r':
						str.push_back('\r');
						break;
					case 't':
						str.push_back('\t');
						break;
					default:
						str.push_back(ch);
				}
			} else
				str.push_back(ch);
		}

		return str;
	}

	Number Parser::lexNumber()
	{
		Number num = 0;
		int ch = getch();

		int sign; // znamenko
		if(ch == '-') {
			sign = -1;
			ch = getch();
		} else
			sign = 1;

		if(!std::isdigit(ch))
			parseError("after sign must be numbers");

		/* celociselna cast */
		while(*input && std::isdigit(ch)) {
			num *= 10;
			num += ch - '0';
			ch = getch();
		}

		/* desetinna cast */
		if(ch == '.') {
			double lvl = 0.1;
			if(!std::isdigit(ch = getch()))
				parseError("after decimal point must be numbers");

			while(*input && isdigit(ch)) {
				num += (ch - '0') * lvl;
				lvl /= 10;
				ch = getch();
			}
		}

		/* exponent */
		if(ch == 'e' || ch == 'E') {
			ch = getch();
			int exp = 0;
			int expSign = 1;
			if(ch == '-') {
				expSign = -1;
				ch = getch();
			} else if(ch == '+') {
				expSign = 1;
				ch = getch();
			}

			if(!std::isdigit(ch)) 
				parseError("bad format of exponent");

			while(*input && isdigit(ch)) {
				exp *= 10;
				exp += ch - '0';
				ch = getch();
			}
			retch(ch);

			return num * sign * std::pow(10, exp*expSign);
		} else {
			retch(ch);
			return num * sign;
		}
	}


	void Parser::lex()
	{
		skipSpaces();
		static const std::string allowedChars = "{}:,[]"; // povolene znaky

		int ch = getch();
		if(!input->good())
			token.type = tEOF;
		else if(ch == '"') { // pocatek retezce
			token.type = tSTRING;
			token.attr.string = lexString();
		} else if(ch == '-' || std::isdigit(ch)) { // pocatek cisla
			retch(ch);
			token.type = tNUMBER;
			token.attr.number = lexNumber();
		} else if(std::isalpha(ch)) { // klicove slovo?
			std::string keyword;
			/* nacteni slova */
			while(*input && std::isalpha(ch)) {
				keyword.push_back(ch);
				ch = getch();
			}
			retch(ch);

			/* a kontrola jestli je klicove */
			if(keyword == "true")
				token.type = kTRUE;
			else if(keyword == "false")
				token.type = kFALSE;
			else if(keyword == "null")
				token.type = kNULL;
			else
				parseError("Unknown keyword " + keyword);
		} else if(allowedChars.find(ch) != std::string::npos) // povoleny znak
			token.type = (TokenType)ch;
		else {
			std::stringstream s;
			s << "Unknown character " << (char)ch << " (" << (int)ch << ")";
			parseError(s.str());
		}
	}

	Value Parser::parseValue()
	{
		lex();
		Value val;
		switch(token.type) {
			case tSTRING:
				val = token.attr.string;
				break;
			case tNUMBER: 
				val = token.attr.number;
				break;
			case kTRUE:
				val = true;
				break;
			case kFALSE:
				val = false;
				break;
			case kNULL:
				/* vychozi hodnota pro Value je null, takze s nim nebudeme delat nic */
				break;
			case '{': // pocatek objektu
				{
					Object object;
					while(true) {
						lex();
						expectToken(tSTRING);
						String key = token.attr.string;

						lex();
						expectToken((TokenType)':');
						object[key] = parseValue();

						lex();
						if(token.type == '}')
							break;
						else if(token.type == ',')
							continue; 
						else
							syntaxError("After value in object must be ',' or '}', not "
													+ tokenType2Name(token.type) + "!");
					}
					val = object;
				}
				break;
			case '[': // pocatek pole
				{
					Array array;
					while(true) {
						array.push_back(parseValue());
						lex();
						if(token.type == ']')
							break;
						else if(token.type == ',')
							continue;
						else
							syntaxError("After value in array must be ',' or ']', not "
													+ tokenType2Name(token.type) + "!");
					}
					val = array;
				}
				break;
			default:
				syntaxError("Expected value, not " + tokenType2Name(token.type));
				break;
		}
		val.setLine(line);
		return val;
	}

	Value Parser::parse(std::istream &in)
	{
		input = &in;
		line = 1;
		const Value val = parseValue();
		lex();
		expectToken(tEOF); // meli bychom byt na konci souboru
		return val;
	}
}
