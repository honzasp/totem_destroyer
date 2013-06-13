#ifndef HAVE_json_value_hpp
#define HAVE_json_value_hpp
#include <string>
#include <map>
#include <vector>
#include "exceptions.hpp"

namespace json {
	enum ValueType {
		TYPE_OBJECT,
		TYPE_ARRAY,
		TYPE_STRING,
		TYPE_NUMBER,
		TYPE_BOOLEAN,
		TYPE_NULL
	};

	const std::string typeNames[] = {
		"object",
		"array",
		"string",
		"number",
		"boolean",
		"null"
	};

	class Value;
	/* implementation of  types in C++ */
	typedef std::string String;
	typedef std::map<String, Value> Object;
	typedef std::vector<Value> Array;
	typedef double Number;
	typedef bool Boolean;

	extern const Value NullValue; // helper - value Null

	/** JSON hodnota */
	class Value {
		ValueType mType; ///< Typ hodnoty
		size_t mLine; ///< Cislo radku kde se tato hodnota vyskytla (0 je neznamy)
		union {
			Object *object;
			Array *array;
			String *string;
			Number number;
			Boolean boolean;
		} mValue; ///< Samotna hodnota

		/** Smazani stareho obsahu. */
		void deleteOld();

		/** Kopirovani obsahu. */
		void copyFrom(const Value &other);
	public:
		/** Je ocekavan typ type.
			* @throw TypeError Typ neni spravny 
			*/
		void expectedType(ValueType type) const;

		/** Vychozi typ je null. */
		Value(): mType(TYPE_NULL), mLine(0) { };

		/** Vychozi hodnota pro typ */
		Value(ValueType type);

		/** Kopirovani */
		Value(const Value &other): mType(other.mType), mLine(0) { copyFrom(other); }

		/** Prirazeni */
		Value &operator=(const Value &other);

		/** Smazani obsahu */
		~Value() { deleteOld(); }


		/** Nastaveni na objekt */
		explicit Value(const Object &object): mType(TYPE_NULL), mLine(0) { *this = object; }

		/** Nastaveni na pole */
		explicit Value(const Array &array): mType(TYPE_NULL), mLine(0) { *this = array; }

		/** Nastaveni na retezec */
		explicit Value(const String &string): mType(TYPE_NULL), mLine(0) { *this = string; }

		/** Nastaveni na Ceckovy retezec */
		explicit Value(const char *string): mType(TYPE_NULL), mLine(0) { *this = String(string); }

		/** Nastaveni na cislo */
		explicit Value(const Number number): mType(TYPE_NULL), mLine(0) { *this = number; }

		/** Nastaveni na boolean */
		explicit Value(const Boolean boolean): mType(TYPE_NULL), mLine(0) { *this = boolean; }


		/** Radek na kterem byla tato hodnota uvedena */
		size_t getLine() { return mLine; }

		/** Nastavi radek na kterem byla tato hodnota uvedena */
		void setLine(size_t line) { mLine = line; }


		/** Nastaveni na objekt */
		Value &operator=(const Object &object);

		/** Nastaveni na pole */
		Value &operator=(const Array &object);

		/** Nastaveni na retezec */
		Value &operator=(const String &object);

		/** Nastaveni na cislo */
		Value &operator=(const Number &object);

		/** Nastaveni na boolean */
		Value &operator=(const Boolean &object);


		/** Pretypovani na objekt */
		const Object &obj() const;

		/** Pretypovani na pole */
		const Array &ary() const;

		/** Pretypovani na retezec */
		const String &str() const;

		/** Pretypovani na cislo */
		const Number &num() const;

		/** Pretypovani na boolean */
		Boolean bol() const;


		/** Je objekt? */
		bool isObject() { return mType == TYPE_OBJECT; }

		/** Je pole? */
		bool isArray() { return mType == TYPE_ARRAY; }

		/** Je retezec? */
		bool isString() { return mType == TYPE_STRING; }

		/** Je cislo? */
		bool isNumber() { return mType == TYPE_NUMBER; }

		/** Je boolean? */
		bool isBoolean() { return mType == TYPE_BOOLEAN; }

		/** Je null? */
		bool isNull() { return mType == TYPE_NULL; }


		/** Ziskani prvku pole (ocekava pole) */
		Value &operator[](size_t index); 

		/** Ziskani prvku pole (ocekava pole) */
		const Value &operator[](size_t index) const; 

		/** Ziskani prvku pole (ocekava pole) */
		Value &operator[](int index) { return (*this)[(size_t)index]; }

		/** Ziskani prvku pole (ocekava pole) */
		const Value &operator[](int index) const { return (*this)[(size_t)index]; } 


		/** Ziskani prvku objektu */
		Value &operator[](const String &key); // for objects

		/** Ziskani prvku objektu */
		const Value &operator[](const String &key) const; 

		/** Ziskani prvku objektu */
		Value &operator[](const char *key) { return (*this)[String(key)]; }

		/** Ziskani prvku objektu */
		const Value &operator[](const char *key) const { return (*this)[String(key)]; }; 


		/** Ziskani prvku objektu.
		* Ziska prvek objektu s klicem key. 
		*
		* @param key Klic prvku objektu
		* @param alt Vychozi hodnota
		* @return Prvek objektu nebo vychozi hodnotu pokud tam prislusny prvek neni
		*/
		Value &get(const String &key, Value &alt); // if key not found return alt

		/** Ziskani prvku objektu.
		* Ziska prvek objektu s klicem key. 
		*
		* @param key Klic prvku objektu
		* @param alt Vychozi hodnota
		* @return Prvek objektu nebo vychozi hodnotu pokud tam prislusny prvek neni
		*/
		const Value &get(const String &key, const Value &alt) const;

		/** Ma objekt neco s klicem key? */
		bool have(const String &key) const;

		/** V objektu je ocekavan prvek key.
		*
		* @param key Prislusny klic
		* @throw DataError Prvek s klicem key v objektu neni
		*/
		void required(const String &key) const; // throw DataError if object haven't member 'key'

		/** Jaky je typ? */
		ValueType getType() const { return mType; }
	};
}
#endif
