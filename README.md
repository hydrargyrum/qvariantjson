qvariantjson is yet another QVariant to JSON and JSON to QVariant 
implementation.

Advantages:
* Public domain
* Easy to use, 500 lines of code, with no JsonParserFactoryFactory bullshit
* For Qt4
* Able to report JSON parsing errors
* 150 lines of test code, also testing parse errors
* Handles 64bits integers (unsigned and signed) and double-precision floats

Disadvantages:
* For API simplicity, parses JSON's "null" and "false" as QVariant(false)
* Uses QVariant::toString() on custom types
* No warranty

You can just copy-paste the .cpp/.h files in your project, or you can 
use qvariantjson as a lib if you insist.
