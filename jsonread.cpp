#include <QVariant>
#include <QtDebug>

#define SPACECASES ' ': case '\n': case '\t': case '\r'

/** \brief Parses \a data and return a QVariant.
Error handling: returns an Invalid QVariant */
QVariant jsonParseValue(const QByteArray &data);

/** \brief Like parseValue(QByteArray)
Returns "\0" in case of success, else returns string location where there's an error.
*/
const char *jsonParseValue(const char *s, QVariant *out);

namespace {
/* for all functions:
return value is place in buffer where it stops
*out is an invalid QVariant() if a parse error
*/
const char *parseBoolean(const char *s, QVariant *out);
const char *parseNumber(const char *s, QVariant *out);
const char *parseString(const char *s, QVariant *out);
const char *parseArray(const char *s, QVariant *out);
const char *parseDict(const char *s, QVariant *out);
const char *chompWhitespace(const char *s);
}

QVariant jsonParseValue(const QByteArray &data) {
	QVariant res;
	const char *s = jsonParseValue(data.constData(), &res);
	s = chompWhitespace(s);

	if (!res.isValid()) {
		if (!*s)
			qDebug() << "short read";
		else
			qDebug() << "parse error:" << s;
	} else if (*s != '\0') {
		qDebug() << "trailing data left:" << s;
	}
	return res;
}

const char *jsonParseValue(const char *s, QVariant *out) {
	retry:
	switch (*s) {
	case SPACECASES:
		++s;
		goto retry;
	case 't': case 'f': case 'n':
		return parseBoolean(s, out);
	case '0': case '1': case '2':
	case '3': case '4': case '5':
	case '6': case '7': case '8':
	case '9': case '-': case '.':
	case 'e': case '+': case 'E':
		return parseNumber(s, out);
	case '\'': case '"':
		return parseString(s, out);
	case '{':
		return parseDict(s, out);
	case '[':
		return parseArray(s, out);
	default:
		*out = QVariant();
		return s;
	}
}


namespace {
const char *chompWhitespace(const char *s) {
	do {
		switch (*s) {
		case SPACECASES:
			++s;
			break;
		default:
			return s;
		}
	} while (true);
}

// next functions: make invalid QVariant at start, return in middle of function means error
// next functions: don't chomp whitespace before and after the parsed value
const char *parseBoolean(const char *s, QVariant *out) {
	*out = QVariant();

	QByteArray sample = QByteArray::fromRawData(s, 7);
	if (sample.startsWith("true")) {
		switch (sample.at(4)) {
		case '\0': case ',': case ']': case '}':
		case SPACECASES:
			*out = QVariant(true);
			return (s + 4);
		}
	} else if (sample.startsWith("false")) {
		switch (sample.at(5)) {
		case '\0': case ',': case ']': case '}':
		case SPACECASES:
			*out = QVariant(false);
			return (s + 5);
		}
	} else if (sample.startsWith("null")) {
		switch (sample.at(4)) {
		case '\0': case ',': case ']': case '}':
		case SPACECASES:
			*out = QVariant(false); // TODO: QVariant()
			return (s + 4);
		}
	}

	return s;
}

const char *parseNumber(const char *s, QVariant *out) {
	*out = QVariant();

	const char *start = s;
	bool finished = false;
	do {
		switch (*s) {
		case SPACECASES:
		case ']': case '}': case ',': case '\0':
			finished = true;
			break;
		case '0': case '1': case '2':
		case '3': case '4': case '5':
		case '6': case '7': case '8':
		case '9': case '-': case '+':
		case '.': case 'e': case 'E':
			++s;
			break;
		default:
			return s;
		}
	} while (!finished);

	QByteArray bytes = QByteArray(start, s - start);
	QVariant number(bytes);
	bool isLongLong;
	bool converted;
	bytes.toLongLong(&isLongLong);
	if (isLongLong)
		converted = number.convert(QVariant::LongLong);
	else {
		bytes.toULongLong(&isLongLong);
		if (isLongLong)
			converted = number.convert(QVariant::ULongLong);
		else
			converted = number.convert(QVariant::Double);
	}

	if (converted)
		*out = number;

	return s;
}

const char *parseString(const char *s, QVariant *out) {
	*out = QVariant();

	char sep = *s;
	if (sep != '\'' && sep != '"')
		return s;

	++s;
	QString res;

	bool backslash = false;
	bool finished = false;
	do {
		switch (*s) {
		case '\0':
			return s;
		case '\\':
			if (!backslash)
				backslash = true;
			else {
				res += '\\';
				backslash = false;
			}
			++s;
			break;
		case '\'': case '"':
			if (*s != sep || backslash) {
				backslash = false;
				res += *s++;
			} else {
				++s;
				finished = true;
			}
			break;
		case 'b':
			if (!backslash)
				res += *s;
			else
				res += '\b';

			++s;
			backslash = false;
			break;
		case 'f':
			if (!backslash)
				res += *s;
			else
				res += '\f';

			++s;
			backslash = false;
			break;
		case 'n':
			if (!backslash)
				res += *s;
			else
				res += '\n';

			++s;
			backslash = false;
			break;
		case 'r':
			if (!backslash)
				res += *s;
			else
				res += '\r';

			++s;
			backslash = false;
			break;
		case 't':
			if (!backslash)
				res += *s;
			else
				res += '\t';

			++s;
			backslash = false;
			break;

		case 'u':
			if (!backslash)
				res += *s++;
			else {
				++s;

				bool parsed;
				ushort cpoint = QByteArray(s, 4).toUShort(&parsed, 16);
				if (parsed)
					res += QChar(cpoint);
				else
					return s;

				s += 4;
			}

			backslash = false;
			break;
		case '/':
			res += *s++;
			backslash = false;
			break;
		default:
			res += *s++;
			backslash = false;
		}
	} while (!finished);

	*out = res;

	return s;
}

const char *parseArray(const char *s, QVariant *out) {
	*out = QVariant();

	++s;
	QList<QVariant> res;
	QVariant current;

	bool finished = false;
	bool needsComma = false;
	do {
		switch (*s) {
		case '\0':
			return s;
		case SPACECASES:
			++s;
			break;
		case ',':
			if (!needsComma)
				return s;
			needsComma = false;
			++s;
			break;
		case ']':
			++s;
			finished = true;
			break;
		default:
			if (needsComma)
				return s;

			s = jsonParseValue(s, &current);
			if (current.isValid())
				res.append(current);
			else
				return s;
			needsComma = true;
		}
	} while (!finished);

	*out = res;

	return s;
}

const char *parseDict(const char *s, QVariant *out) {
	*out = QVariant();

	++s;

	QMap<QString, QVariant> res;
	QVariant key;
	QVariant value;

	enum {Key, Colon, Value, ValueFinish, End} state = Key;
	do {
		if (state == Key) {
			switch (*s) {
			case '\0':
				return s;
			case SPACECASES:
				++s;
				break;
			case '}':
				state = End;
				++s;
				break;
			case '\'': case '"':
				s = parseString(s, &key);
				if (key.isValid())
					state = Colon;
				else
					return s;
				break;
			default:
				return s;
			}
		} else if (state == Value) {
			s = jsonParseValue(s, &value);
			if (value.isValid()) {
				state = ValueFinish;
				res.insert(key.toString(), value);
			} else
				return s;
		} else {
			switch (*s) {
			case ':':
				if (state == Colon) {
					state = Value;
					++s;
				} else
					return s;
				break;
			case ',':
				if (state == ValueFinish) {
					state = Key;
					++s;
				} else
					return s;
				break;
			case SPACECASES:
				++s;
				break;
			case '}':
				if (state == ValueFinish)
					++s;
				else
					return s;
				state = End;
				break;
			default:
				return s;
			}
		}
	} while (state != End);

	*out = res;

	return s;
}

} // unnamed namespace
