#ifndef GTEST_REPORTER_EH
#define GTEST_REPORTER_EH

#include <gtest/gtest.h>
#include <unistd.h>
using namespace testing::internal;

namespace testing {
	class EHReporter : public EmptyTestEventListener {
    public:
        explicit EHReporter(const char* output_file);

        void OnTestIterationEnd(const UnitTest& unit_test, int iteration) override;
        void ListTestsMatchingFilter(const std::vector<TestSuite*>& test_suites);
        void OnTestSuiteStart(const TestSuite& /*test_suite*/) override;
        void OnTestStart(const TestInfo& test_info) override;
        void OnTestEnd(const TestInfo& /*test_info*/) override;

        // Prints an XML summary of all unit tests.
        static void PrintXmlTestsList(std::ostream* stream,
            const std::vector<TestSuite*>& test_suites, std::vector<std::vector<std::string>> capt_stdout);

    private:
        // Is c a whitespace character that is normalized to a space character
        // when it appears in an XML attribute value?
        static bool IsNormalizableWhitespace(char c) {
            return c == 0x9 || c == 0xA || c == 0xD;
        }

        // May c appear in a well-formed XML document?
        static bool IsValidXmlCharacter(char c) {
            return IsNormalizableWhitespace(c) || c >= 0x20;
        }

        // Returns an XML-escaped copy of the input string str.  If
        // is_attribute is true, the text is meant to appear as an attribute
        // value, and normalizable whitespace is preserved by replacing it
        // with character references.
        static std::string EscapeXml(const std::string& str, bool is_attribute);

        // Returns the given string with all characters invalid in XML removed.
        static std::string RemoveInvalidXmlCharacters(const std::string& str);

        // Convenience wrapper around EscapeXml when str is an attribute value.
        static std::string EscapeXmlAttribute(const std::string& str) {
            return EscapeXml(str, true);
        }

        // Convenience wrapper around EscapeXml when str is not an attribute value.
        static std::string EscapeXmlText(const char* str) {
            return EscapeXml(str, false);
        }

        // Verifies that the given attribute belongs to the given element and
        // streams the attribute as XML.
        static void OutputXmlAttribute(std::ostream* stream,
            const std::string& element_name,
            const std::string& name,
            const std::string& value);

        // Streams an XML CDATA section, escaping invalid CDATA sequences as needed.
        static void OutputXmlCDataSection(::std::ostream* stream, const char* data);

        // Streams an XML representation of a TestInfo object.
        static void OutputXmlTestInfo(::std::ostream* stream,
            const char* test_suite_name,
            const TestInfo& test_info, int iteration, std::vector<std::string> capt_stdout);

        // Prints an XML representation of a TestSuite object
        static void PrintXmlTestSuite(::std::ostream* stream,
            const TestSuite& test_suite, int iteration, std::vector<std::vector<std::string>> capt_stdout);

        // Prints an XML summary of unit_test to output stream out.
        static void PrintXmlUnitTest(::std::ostream* stream,
            const UnitTest& unit_test, std::vector<std::vector<std::string>> capt_stdout);

        // Produces a string representing the test properties in a result as space
        // delimited XML attributes based on the property key="value" pairs.
        // When the std::string is not empty, it includes a space at the beginning,
        // to delimit this attribute from prior attributes.
        static std::string TestPropertiesAsXmlAttributes(const TestResult& result);

        // Streams an XML representation of the test properties of a TestResult
        // object.
        static void OutputXmlTestProperties(std::ostream* stream,
            const TestResult& result);

        // The output file.
        const std::string output_file_;

        std::vector<std::vector<std::string>> captured_stdout; //Test stdout reference to provide <stdout> XML-Tag to Junit
        std::vector<std::string>* testsuite_stdout_vec = nullptr;
	};
}

#endif  // GTEST_REPORTER_EH
