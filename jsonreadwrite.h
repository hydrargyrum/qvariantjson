#include <QVariant>
#include <QByteArray>

/** \brief Parses \a data and return a QVariant.
Error handling: returns an Invalid QVariant */
QVariant jsonParseValue(const QByteArray &data);

/** \brief Like jsonParseValue(QByteArray)
Returns "\0" in case of success, else returns string location where there's an $
*/
const char *jsonParseValue(const char *s, QVariant *out);


QByteArray writeJson(const QVariant &v);
