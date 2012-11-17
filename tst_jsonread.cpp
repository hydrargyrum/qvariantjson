#include <QtTest>
#include "jsonreadwrite.h"

class TestJsonRead : public QObject {
	Q_OBJECT

private slots:
	void testNumbers() {
		QCOMPARE(jsonParseValue("  123  "), QVariant(123));
		QCOMPARE(jsonParseValue("-456.123"), QVariant(-456.123));
		// test 64 bits integers
		QCOMPARE(jsonParseValue("18446744073709551615"), QVariant(18446744073709551615ULL));
		QCOMPARE(jsonParseValue("-9223372036854775808"), QVariant(-9223372036854775808LL));
	}

	void testBools() {
		QCOMPARE(jsonParseValue("true"), QVariant(true));
		QCOMPARE(jsonParseValue(" false"), QVariant(false));
		// "null" not handled
	}

	void testStrings() {
		QCOMPARE(jsonParseValue("\"\""), QVariant(QString()));
		QCOMPARE(jsonParseValue("\"aaa b'bb [] {} 123\""), QVariant("aaa b'bb [] {} 123"));
		QCOMPARE(jsonParseValue("\"a-\\n-\\ -\\\"-\\\\\\\"-b-\\u00E9\""),
		         QVariant(QString::fromLatin1("a-\n- -\"-\\\"-b-\xe9").normalized(QString::NormalizationForm_KC)));
	}

	void testListsMaps() {
		QCOMPARE(QVariant(QVariantList()), jsonParseValue("[  ]"));

		{
			QVariantList list;
			list << 123;
			QCOMPARE(jsonParseValue("[123]"), QVariant(list));
		}

		{
			QVariantList list;
			list << 123 << 456;
			QCOMPARE(jsonParseValue("[123, 456]"), QVariant(list));
		}

		{
			QVariantList list;
			list << 123 << QVariant(QVariantList() << 456 << 789);
			QCOMPARE(jsonParseValue("[123, [456  , 789]]"), QVariant(list));
		}

		QCOMPARE(jsonParseValue("{}"), QVariant(QVariantMap()));

		{
			QVariantList list;
			QVariantMap map;

			list << 789 << "foo-\"-bar";
			map["y"] = 2e3; map["x"] = -42;
			list << map;
			map.clear();
			map["x"] = 123; map["y"] = 456; map["z"] = list;

			QCOMPARE(QVariant(map), jsonParseValue(" {\"x\":123, \"y\" : 456, \"z\": [789, \"foo-\\\"-bar\", {\"x\": 1, \"y\": 2e3 , \"x\" : -42 } ]} "));
		}
	}

	void testExtraSyntax() {
		{
			QVariantList list;
			list << 1;
			QCOMPARE(QVariant(list), jsonParseValue("[1, ]"));
		}

		{
			QVariantMap map;
			map["a"] = 42;
			QCOMPARE(QVariant(map), jsonParseValue("{\"a\": 42, }"));
		}

		{
			QVariantList list;
			QVariantMap map;
			map["d"] = "e";
			list << "a" << "b\"c" << map;
			QCOMPARE(QVariant(list), jsonParseValue("['a', 'b\"c', {'d': \"e\"}]"));
		}

	}

	void testErrors() {
		QCOMPARE(QVariant(), jsonParseValue("\""));
		QCOMPARE(QVariant(""), jsonParseValue("\"\"xyz"));
		QCOMPARE(QVariant(), jsonParseValue("1a"));
		QCOMPARE(QVariant(), jsonParseValue("a"));
		QCOMPARE(QVariant(), jsonParseValue("*"));
		QCOMPARE(QVariant(), jsonParseValue("["));
		QCOMPARE(QVariant(), jsonParseValue("[,"));
		QCOMPARE(QVariant(), jsonParseValue("[1,"));
		QCOMPARE(QVariant(), jsonParseValue("[1,,1]"));
		QCOMPARE(QVariant(), jsonParseValue("[1 1]"));
		QCOMPARE(QVariant(), jsonParseValue("[, 1]"));
		QCOMPARE(QVariant(), jsonParseValue("{:"));
		QCOMPARE(QVariant(), jsonParseValue("{\"a"));
		QCOMPARE(QVariant(), jsonParseValue("{\"a\""));
		QCOMPARE(QVariant(), jsonParseValue("{\"a\":"));
		QCOMPARE(QVariant(), jsonParseValue("{\"a\":}"));
		QCOMPARE(QVariant(), jsonParseValue("{\"a\":,}"));
		QCOMPARE(QVariant(), jsonParseValue("{\"a\": 1, 1: 3}"));
		QCOMPARE(QVariant(), jsonParseValue("{\"a\": 1,, \"x\": 2}"));

		qDebug() << "Note: the above QDEBUGs are not ERRORs";
	}
};

void do_TestJsonRead() {
	TestJsonRead obj;
	QTest::qExec(&obj);
}

#include "tst_jsonread.moc"
