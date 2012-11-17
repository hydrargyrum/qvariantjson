#include <QtEndian>
#include <QVariant>

QByteArray writeJson(const QVariant &v);

namespace {
/* internal */
void writeJson(const QVariant &v, QByteArray *out);
}

QByteArray writeJson(const QVariant &v) {
	QByteArray ret;
	writeJson(v, &ret);
	return ret;
}

namespace {

template <typename T>
void writeJsonList(const T &list, QByteArray *out) {
	out->append("[");
	bool first = true;
	foreach (const QVariant &v, list) {
		if (!first)
			out->append(",");
		writeJson(v, out);
		first = false;
	}
	out->append("]");
}

template <typename T>
void writeJsonDict(const T &dict, QByteArray *out) {
	out->append("{");
	bool first = true;
	for (typename T::const_iterator it = dict.begin(); it != dict.end(); ++it)  {
		if (!first)
			out->append(",");
		writeJson(it.key(), out);
		out->append(":");
		writeJson(it.value(), out);
		first = false;
	}
	out->append("}");
}

void writeJson(const QString &s, QByteArray *out) {
	//QByteArray utfstring = s.toUtf8();
	//const char *bytes = utfstring.constData();
	const QChar *c = s.unicode();
	out->append('"');
	bool finished = false;
	while (!finished) {
		switch (c->unicode()) {
		case '\0':
			finished = true;
			break;
		case '\b':
			out->append("\\b");
			break;
		case '\f':
			out->append("\\f");
			break;
		case '\n':
			out->append("\\n");
			break;
		case '\r':
			out->append("\\r");
			break;
		case '\t':
			out->append("\\t");
			break;
		case '\\':
			out->append("\\\\");
			break;
		case '"':
			out->append("\\\"");
			break;
		default:
			if (c->unicode() < 127)
				out->append(c->unicode());
			else {
				out->append("\\u");
				ushort ubytes = qToBigEndian(c->unicode());
				out->append(QByteArray((char*) &ubytes, 2).toHex());
			}
		}
		c++;
	}
	out->append('"');
}

void writeJson(const QVariant &v, QByteArray *out) {
	switch (v.type()) {
	case QVariant::Invalid:
		out->append("null");
		break;
	case QVariant::Bool:
		if (v.toBool())
			out->append("true");
		else
			out->append("false");
		break;
	case QVariant::ULongLong: case QVariant::UInt:
		out->append(QByteArray::number(v.toULongLong()));
		break;
	case QVariant::LongLong: case QVariant::Int:
		out->append(QByteArray::number(v.toLongLong()));
		break;
	case QVariant::Double:
		out->append(QByteArray::number(v.toDouble()));
		break;
	case QVariant::List:
		writeJsonList(v.toList(), out);
		break;
	case QVariant::Map:
		writeJsonDict(v.toMap(), out);
		break;
	case QVariant::Hash:
		writeJsonDict(v.toHash(), out);
		break;
	case QVariant::String:
	default:
		writeJson(v.toString(), out);
	}
}

} // namespace
