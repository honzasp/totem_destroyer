#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include "value.hpp"

namespace json {
	const Value NullValue;

	void Value::deleteOld()
	{
		switch(mType) {
			/* objekt, pole a retezec se musi smazat */
			case TYPE_OBJECT:
				delete mValue.object;
				break;
			case TYPE_ARRAY:
				delete mValue.array;
				break;
			case TYPE_STRING:
				delete mValue.string;
				break;
			default:
				break;
		}
	}

	void Value::copyFrom(const Value &other)
	{
		mLine = other.mLine;
		switch(mType) {
			case TYPE_OBJECT:
				mValue.object = new Object;
				*(mValue.object) = *(other.mValue.object);
				break;
			case TYPE_ARRAY:
				mValue.array = new Array;
				*(mValue.array) = *(other.mValue.array);
				break;
			case TYPE_STRING:
				mValue.string = new String;
				*(mValue.string) = *(other.mValue.string);
				break;
			case TYPE_NUMBER:
				mValue.number = other.mValue.number;
				break;
			case TYPE_BOOLEAN:
				mValue.boolean = other.mValue.boolean;
				break;
			default:
				break;
		}
	}

	Value &Value::operator=(const Value &other)
	{
		if(&other != this) {
			deleteOld();
			mType = other.mType;
			copyFrom(other);
		}
		return *this;
	}

	Value::Value(ValueType type): mLine(0)
	{
		mType = type;
		switch(mType) {
			case TYPE_OBJECT:
				mValue.object = new Object;
				break;
			case TYPE_ARRAY:
				mValue.array = new Array;
				break;
			case TYPE_STRING:
				mValue.string = new String;
				break;
			case TYPE_NUMBER:
				mValue.number = 0.0;
				break;
			case TYPE_BOOLEAN:
				throw TypeError("Is it true or false? How I can initialize boolean?");
				break;
			default:
				break;
		}
	}

	Value &Value::operator=(const Object &object)
	{
		if(mType == TYPE_OBJECT)
			*mValue.object = object;
		else {
			deleteOld();
			mType = TYPE_OBJECT;
			mValue.object = new Object(object);
		}
		return *this;
	}

	Value &Value::operator=(const Array &array)
	{
		if(mType == TYPE_ARRAY)
			*mValue.array = array;
		else {
			deleteOld();
			mType = TYPE_ARRAY;
			mValue.array = new Array(array);
		}
		return *this;
	}

	Value &Value::operator=(const String &string)
	{
		if(mType == TYPE_STRING)
			*mValue.string = string;
		else {
			deleteOld();
			mType = TYPE_STRING;
			mValue.string = new String(string);
		}
		return *this;
	}

	Value &Value::operator=(const Number &number)
	{
		deleteOld();
		mType = TYPE_NUMBER;
		mValue.number = number;
		return *this;
	}

	Value &Value::operator=(const Boolean &boolean)
	{
		deleteOld();
		mType = TYPE_BOOLEAN;
		mValue.boolean = boolean;
		return *this;
	}

	void Value::expectedType(ValueType type) const
	{
		if(mType != type) {
			if(mLine != 0) {
				std::stringstream s;
				s << "Expected " << typeNames[type] << ", not " << typeNames[mType]
					<< " on line " << mLine;
				throw TypeError(s.str());
			} else
				throw TypeError("Expected " + typeNames[type] + ", not " + typeNames[mType]);
		}
	}

	const Object &Value::obj() const
	{
		expectedType(TYPE_OBJECT);
		return *mValue.object;
	}

	const Array &Value::ary() const
	{
		expectedType(TYPE_ARRAY);
		return *mValue.array;
	}

	const String &Value::str() const
	{
		expectedType(TYPE_STRING);
		return *mValue.string;
	}

	const Number &Value::num() const
	{
		expectedType(TYPE_NUMBER);
		return mValue.number;
	}

	Boolean Value::bol() const
	{
		if(mType == TYPE_NULL)
			return false;
		expectedType(TYPE_BOOLEAN);
		return mValue.boolean;
	}

	Value &Value::operator[](size_t index)
	{
		expectedType(TYPE_ARRAY);
		return (*mValue.array).at(index);
	}

	const Value &Value::operator[](size_t index) const
	{
		expectedType(TYPE_ARRAY);
		return (*mValue.array).at(index);
	}

	Value &Value::operator[](const String &key)
	{
		expectedType(TYPE_OBJECT);
		return (*mValue.object)[key];
	}

	const Value &Value::operator[](const String &key) const
	{
		expectedType(TYPE_OBJECT);
		return (*mValue.object)[key];
	}

	bool Value::have(const String &key) const
	{
		expectedType(TYPE_OBJECT);
		return (*mValue.object).find(key) != (*mValue.object).end();
	}

	Value &Value::get(const String &key, Value &alt)
	{
		expectedType(TYPE_OBJECT);
		Object::iterator it = (*mValue.object).find(key);
		if(it == (*mValue.object).end())
			return alt;
		else
			return it->second;
	}

	const Value &Value::get(const String &key, const Value &alt) const
	{
		expectedType(TYPE_OBJECT);
		Object::iterator it = (*mValue.object).find(key);
		if(it == (*mValue.object).end())
			return alt;
		else
			return it->second;
	}

	void Value::required(const String &key) const
	{
		expectedType(TYPE_OBJECT);
		if(!have(key)) {
			std::stringstream s;
			if(mLine != 0) 
				s << "Object on line " << mLine << " haven't required member " << key;
			else
				s << "Member " << key << " is required!";
			throw DataError(s.str());
		}
	}

}
