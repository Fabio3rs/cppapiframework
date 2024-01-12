#include "projstdafx.hpp"

#include "Json2StructTest.hpp"
#include "../src/JSON/StructParserMacros.hpp"

namespace JSONStructParser {
MAKE_DISABLE_SET_STRUCT(Json2StructTest::AnotherStruct);
MAKE_START_OBJECT_SPECIALIZATION(Json2StructTest::AnotherStruct);
// NOLINTNEXTLINE
MAKE_FIELD_LIST_JS(Json2StructTest::AnotherStruct, name, lastname);
// NOLINTNEXTLINE
MAKE_FIELD_LIST_JS(Json2StructTest::TestJs, anotherval, val, dataarr, intvalue, userdata,
                   otherUsers, algumacoisa, multikeyvalue);
} // namespace JSONStructParser

auto Json2StructTest::Helper::makeParserForTestJs(TestJs &data)
    -> Poco::SharedPtr<JSONStructParser::StructParser> {
    return new JSONStructParser::StructParser(
        std::make_unique<JSONStructParser::TemplateStructFiller<TestJs>>(data));
}
