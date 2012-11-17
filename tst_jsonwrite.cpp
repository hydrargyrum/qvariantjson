#include <QtTest>
#include "jsonreadwrite.h"

class TestJsonWrite : public QObject {
	Q_OBJECT

private slots:
	void testNumbers() {
		QCOMPARE(writeJson(QVariant(42)), QByteArray("42"));
		QCOMPARE(writeJson(QVariant(-42)), QByteArray("-42"));
		QCOMPARE(writeJson(QVariant(42.5)), QByteArray("42.5"));

		QCOMPARE(writeJson(QVariant(quint64(0xffffffffffffffff))), QByteArray("18446744073709551615"));
		QCOMPARE(writeJson(QVariant(qint64(0x8000000000000000))), QByteArray("-9223372036854775808"));
	}

	void testConsts() {
		QCOMPARE(writeJson(QVariant()), QByteArray("null"));
		QCOMPARE(writeJson(QVariant(true)), QByteArray("true"));
		QCOMPARE(writeJson(QVariant(false)), QByteArray("false"));
	}

	void testStrings() {
		QCOMPARE(writeJson(QVariant("")), QByteArray("\"\""));
		QCOMPARE(writeJson(QVariant("x")), QByteArray("\"x\""));
		QCOMPARE(writeJson(QVariant("foo\\bar\"baz")), QByteArray("\"foo\\\\bar\\\"baz\""));
		QCOMPARE(writeJson(QVariant("foo'bar")), QByteArray("\"foo'bar\""));
		QCOMPARE(writeJson(QString::fromLatin1("-\xe9\t-")), QByteArray("\"-\\u00e9\\t-\""));
	}

	void testListsMaps() {
		QCOMPARE(writeJson(QVariantList()), QByteArray("[]"));

		{
			QVariantList list;
			list << 42;
			QCOMPARE(writeJson(list), QByteArray("[42]"));
		}

		QCOMPARE(writeJson(QVariantMap()), QByteArray("{}"));

		{
			QVariantMap map;
			map["def"] = 42;
			map["abc"] = true;
			QCOMPARE(writeJson(map), QByteArray("{\"abc\":true,\"def\":42}"));
		}

		{
			QMap<QString,QVariant> map;
			map["x"] = true;
			map["y"] = "789";
			map["z"] = 1234;
			QList<QVariant> list;
			list << 1 << QString::fromLatin1("foo bar' <- quote baz \xe9 <- unicode \t<- tab \" <- dquote") << 1.3
			     << map << 0 << QVariant();
			QCOMPARE(writeJson(list),
			         QByteArray("[1,\"foo bar' <- quote baz \\u00e9 <- unicode \\t<- tab \\\" <- dquote\",1.3,"
			                    "{\"x\":true,\"y\":\"789\",\"z\":1234},0,null]"));
		}
	}
};

void do_TestJsonWrite() {
	TestJsonWrite obj;
	QTest::qExec(&obj);
}

void do_TestJsonRead();

int main() {
	do_TestJsonRead();
	do_TestJsonWrite();
}


#include "tst_jsonwrite.moc"
