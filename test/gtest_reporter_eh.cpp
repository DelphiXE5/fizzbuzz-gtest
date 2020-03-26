#include "gtest_reporter_eh.hpp"
#include <string.h>

namespace testing
{
    EHReporter::EHReporter(const char* output_file)
        : output_file_(output_file) {
        if (output_file_.empty()) {
            GTEST_LOG_(FATAL) << "XML output file may not be null";
        }
    }

    static FILE* OpenFileForWriting(const std::string& output_file) {
        FILE* fileout = nullptr;
        FilePath output_file_path(output_file);
        FilePath output_dir(output_file_path.RemoveFileName());

        if (output_dir.CreateDirectoriesRecursively()) {
            fileout = posix::FOpen(output_file.c_str(), "w");
        }
        if (fileout == nullptr) {
            GTEST_LOG_(FATAL) << "Unable to open file \"" << output_file << "\"";
        }
        return fileout;
    }

    // Called after the unit test ends.
    void EHReporter::OnTestIterationEnd(const UnitTest& unit_test,
        int /*iteration*/) {
        FILE* xmlout = OpenFileForWriting(output_file_);
        std::stringstream stream;
        PrintXmlUnitTest(&stream, unit_test, captured_stdout);
        fprintf(xmlout, "%s", StringStreamToString(&stream).c_str());
        fclose(xmlout);
    }

    void EHReporter::ListTestsMatchingFilter(
        const std::vector<TestSuite*>& test_suites) {
        FILE* xmlout = OpenFileForWriting(output_file_);
        std::stringstream stream;
        PrintXmlTestsList(&stream, test_suites, captured_stdout);
        fprintf(xmlout, "%s", StringStreamToString(&stream).c_str());
        fclose(xmlout);
    }

    void EHReporter::OnTestSuiteStart(const TestSuite&)
    {
        std::vector<std::string> suitvector;
        captured_stdout.push_back(suitvector);
        testsuite_stdout_vec = &captured_stdout.back();
        return;
    }

    void EHReporter::OnTestStart(const TestInfo& test_info)
    {
        CaptureStdout();
        return;
    }

    void EHReporter::OnTestEnd(const TestInfo&)
    {
        std::string content = GetCapturedStdout();
        (*testsuite_stdout_vec).push_back(content);
        int size = (*testsuite_stdout_vec).size();
        for (int i = 0; i < (*testsuite_stdout_vec).size(); i++)
            std::cout << (*testsuite_stdout_vec).at(i) << ' ';
        //printf("From pipe: %s", content.c_str());
        return;
    }

    // Returns an XML-escaped copy of the input string str.  If is_attribute
    // is true, the text is meant to appear as an attribute value, and
    // normalizable whitespace is preserved by replacing it with character
    // references.
    //
    // Invalid XML characters in str, if any, are stripped from the output.
    // It is expected that most, if not all, of the text processed by this
    // module will consist of ordinary English text.
    // If this module is ever modified to produce version 1.1 XML output,
    // most invalid characters can be retained using character references.
    std::string EHReporter::EscapeXml(
        const std::string& str, bool is_attribute) {
        Message m;

        for (size_t i = 0; i < str.size(); ++i) {
            const char ch = str[i];
            switch (ch) {
            case '<':
                m << "&lt;";
                break;
            case '>':
                m << "&gt;";
                break;
            case '&':
                m << "&amp;";
                break;
            case '\'':
                if (is_attribute)
                    m << "&apos;";
                else
                    m << '\'';
                break;
            case '"':
                if (is_attribute)
                    m << "&quot;";
                else
                    m << '"';
                break;
            default:
                if (IsValidXmlCharacter(ch)) {
                    if (is_attribute && IsNormalizableWhitespace(ch))
                        m << "&#x" << String::FormatByte(static_cast<unsigned char>(ch))
                        << ";";
                    else
                        m << ch;
                }
                break;
            }
        }

        return m.GetString();
    }

    // Returns the given string with all characters invalid in XML removed.
    // Currently invalid characters are dropped from the string. An
    // alternative is to replace them with certain characters such as . or ?.
    std::string EHReporter::RemoveInvalidXmlCharacters(
        const std::string& str) {
        std::string output;
        output.reserve(str.size());
        for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
            if (IsValidXmlCharacter(*it))
                output.push_back(*it);

        return output;
    }

    // The following routines generate an XML representation of a UnitTest
    // object.
    // GOOGLETEST_CM0009 DO NOT DELETE
    //
    // This is how Google Test concepts map to the DTD:
    //
    // <testsuites name="AllTests">        <-- corresponds to a UnitTest object
    //   <testsuite name="testcase-name">  <-- corresponds to a TestSuite object
    //     <testcase name="test-name">     <-- corresponds to a TestInfo object
    //       <failure message="...">...</failure>
    //       <failure message="...">...</failure>
    //       <failure message="...">...</failure>
    //                                     <-- individual assertion failures
    //     </testcase>
    //   </testsuite>
    // </testsuites>

    // Formats the given time in milliseconds as seconds.
    std::string FormatTimeInMillisAsSeconds(TimeInMillis ms) {
        ::std::stringstream ss;
        ss << (static_cast<double>(ms) * 1e-3);
        return ss.str();
    }

    static bool PortableLocaltime(time_t seconds, struct tm* out) {
#if defined(_MSC_VER)
        return localtime_s(out, &seconds) == 0;
#elif defined(__MINGW32__) || defined(__MINGW64__)
        // MINGW <time.h> provides neither localtime_r nor localtime_s, but uses
        // Windows' localtime(), which has a thread-local tm buffer.
        struct tm* tm_ptr = localtime(&seconds);  // NOLINT
        if (tm_ptr == nullptr) return false;
        *out = *tm_ptr;
        return true;
#else
        return localtime_r(&seconds, out) != nullptr;
#endif
    }

    // Converts the given epoch time in milliseconds to a date string in the ISO
    // 8601 format, without the timezone information.
    std::string FormatEpochTimeInMillisAsIso8601(TimeInMillis ms) {
        struct tm time_struct;
        if (!PortableLocaltime(static_cast<time_t>(ms / 1000), &time_struct))
            return "";
        // YYYY-MM-DDThh:mm:ss
        return StreamableToString(time_struct.tm_year + 1900) + "-" +
            String::FormatIntWidth2(time_struct.tm_mon + 1) + "-" +
            String::FormatIntWidth2(time_struct.tm_mday) + "T" +
            String::FormatIntWidth2(time_struct.tm_hour) + ":" +
            String::FormatIntWidth2(time_struct.tm_min) + ":" +
            String::FormatIntWidth2(time_struct.tm_sec);
    }

    static const char* const kReservedTestSuitesAttributes[] = {
  "disabled",
  "errors",
  "failures",
  "name",
  "random_seed",
  "tests",
  "time",
  "timestamp"
    };

    // The list of reserved attributes used in the <testsuite> element of XML
// output.
    static const char* const kReservedTestSuiteAttributes[] = {
        "disabled", "errors", "failures", "name", "tests", "time", "timestamp" };

    // The list of reserved attributes used in the <testcase> element of XML output.
    static const char* const kReservedTestCaseAttributes[] = {
        "classname",   "name", "status", "time",  "type_param",
        "value_param", "file", "line" };

    // Use a slightly different set for allowed output to ensure existing tests can
    // still RecordProperty("result") or "RecordProperty(timestamp")
    static const char* const kReservedOutputTestCaseAttributes[] = {
        "classname",   "name", "status", "time",   "type_param",
        "value_param", "file", "line",   "result", "timestamp" };
    
    template <int kSize>
    std::vector<std::string> ArrayAsVector(const char* const (&array)[kSize]) {
        return std::vector<std::string>(array, array + kSize);
    }

    static std::vector<std::string> GetReservedOutputAttributesForElement(
        const std::string& xml_element) {
        if (xml_element == "testsuites") {
            return ArrayAsVector(kReservedTestSuitesAttributes);
        }
        else if (xml_element == "testsuite") {
            return ArrayAsVector(kReservedTestSuiteAttributes);
        }
        else if (xml_element == "testcase") {
            return ArrayAsVector(kReservedOutputTestCaseAttributes);
        }
        else {
            GTEST_CHECK_(false) << "Unrecognized xml_element provided: " << xml_element;
        }
        // This code is unreachable but some compilers may not realizes that.
        return std::vector<std::string>();
    }

    // Streams an XML CDATA section, escaping invalid CDATA sequences as needed.
    void EHReporter::OutputXmlCDataSection(::std::ostream* stream,
        const char* data) {
        const char* segment = data;
        *stream << "<![CDATA[";
        for (;;) {
            const char* const next_segment = strstr(segment, "]]>");
            if (next_segment != nullptr) {
                stream->write(
                    segment, static_cast<std::streamsize>(next_segment - segment));
                *stream << "]]>]]&gt;<![CDATA[";
                segment = next_segment + strlen("]]>");
            }
            else {
                *stream << segment;
                break;
            }
        }
        *stream << "]]>";
    }

    void EHReporter::OutputXmlAttribute(
        std::ostream* stream,
        const std::string& element_name,
        const std::string& name,
        const std::string& value) {
        /*const std::vector<std::string>& allowed_names =
            GetReservedOutputAttributesForElement(element_name);

        GTEST_CHECK_(std::find(allowed_names.begin(), allowed_names.end(), name) != allowed_names.end())
            << "Attribute " << name << " is not allowed for element <" << element_name
            << ">.";*/

        *stream << " " << name << "=\"" << EscapeXmlAttribute(value) << "\"";
    }

    // Prints an XML representation of a TestInfo object.
    void EHReporter::OutputXmlTestInfo(::std::ostream* stream,
        const char* test_suite_name,
        const TestInfo& test_info, int iteration, std::vector<std::string> capt_stdout) {
        const TestResult& result = *test_info.result();
        const std::string kTestsuite = "testcase";

        if (test_info.is_in_another_shard()) {
            return;
        }

        *stream << "    <testcase";
        OutputXmlAttribute(stream, kTestsuite, "name", test_info.name());

        if (test_info.value_param() != nullptr) {
            OutputXmlAttribute(stream, kTestsuite, "value_param",
                test_info.value_param());
        }
        if (test_info.type_param() != nullptr) {
            OutputXmlAttribute(stream, kTestsuite, "type_param",
                test_info.type_param());
        }
        if (GTEST_FLAG(list_tests)) {
            OutputXmlAttribute(stream, kTestsuite, "file", test_info.file());
            OutputXmlAttribute(stream, kTestsuite, "line",
                StreamableToString(test_info.line()));
            *stream << " />\n";
            return;
        }

        OutputXmlAttribute(stream, kTestsuite, "status",
            test_info.should_run() ? "run" : "notrun");
        OutputXmlAttribute(stream, kTestsuite, "result",
            test_info.should_run()
            ? (result.Skipped() ? "skipped" : "completed")
            : "suppressed");
        OutputXmlAttribute(stream, kTestsuite, "time",
            FormatTimeInMillisAsSeconds(result.elapsed_time()));
        OutputXmlAttribute(
            stream, kTestsuite, "timestamp",
            FormatEpochTimeInMillisAsIso8601(result.start_timestamp()));
        OutputXmlAttribute(stream, kTestsuite, "classname", test_suite_name);

        int failures = 0;
        for (int i = 0; i < result.total_part_count(); ++i) {
            const TestPartResult& part = result.GetTestPartResult(i);
            if (part.failed()) {
                if (++failures == 1) {
                    *stream << ">\n";
                }
                const std::string location =
                    internal::FormatCompilerIndependentFileLocation(part.file_name(),
                        part.line_number());
                const std::string summary = location + "\n" + part.summary();
                *stream << "      <failure message=\""
                    << EscapeXmlAttribute(summary.c_str())
                    << "\" type=\"\">";
                const std::string detail = location.substr(0, location.find(":")) + ":" + test_info.name() + ":" + location.substr(location.find(":")+1, location.size()-1) + "\n" + part.message();
                OutputXmlCDataSection(stream, RemoveInvalidXmlCharacters(detail).c_str());
                *stream << "</failure>\n";
            }
        }

        if (failures == 0 && result.test_property_count() == 0) {
            *stream << " />\n";
        }
        else {
            if (failures == 0) {
                *stream << ">\n";
            }
            *stream << "      <system-out>\n##polylith[testStarted\n" << capt_stdout.at(iteration) << "\n##polylith[testFinished\n        </system-out>\n";
            OutputXmlTestProperties(stream, result);
            *stream << "    </testcase>\n";
        }
    }

    // Prints an XML representation of a TestSuite object
    void EHReporter::PrintXmlTestSuite(std::ostream* stream,
        const TestSuite& test_suite, int iteration, std::vector<std::vector<std::string>> capt_stdout) {
        const std::string kTestsuite = "testsuite";
        *stream << "  <" << kTestsuite;
        OutputXmlAttribute(stream, kTestsuite, "name", test_suite.name());
        OutputXmlAttribute(stream, kTestsuite, "tests",
            StreamableToString(test_suite.reportable_test_count()));
        if (!GTEST_FLAG(list_tests)) {
            OutputXmlAttribute(stream, kTestsuite, "failures",
                StreamableToString(test_suite.failed_test_count()));
            OutputXmlAttribute(
                stream, kTestsuite, "disabled",
                StreamableToString(test_suite.reportable_disabled_test_count()));
            OutputXmlAttribute(stream, kTestsuite, "errors", "0");
            OutputXmlAttribute(stream, kTestsuite, "time",
                FormatTimeInMillisAsSeconds(test_suite.elapsed_time()));
            OutputXmlAttribute(
                stream, kTestsuite, "timestamp",
                FormatEpochTimeInMillisAsIso8601(test_suite.start_timestamp()));
            *stream << TestPropertiesAsXmlAttributes(test_suite.ad_hoc_test_result());
        }
        *stream << ">\n";
        for (int i = 0; i < test_suite.total_test_count(); ++i) {
            if (test_suite.GetTestInfo(i)->is_reportable())
                OutputXmlTestInfo(stream, test_suite.name(), *test_suite.GetTestInfo(i), i,capt_stdout.at(iteration));
        }
        *stream << "  </" << kTestsuite << ">\n";
    }

    // Prints an XML summary of unit_test to output stream out.
    void EHReporter::PrintXmlUnitTest(std::ostream* stream,
        const UnitTest& unit_test, std::vector<std::vector<std::string>> capt_stdout) {
        const std::string kTestsuites = "testsuites";

        *stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        *stream << "<" << kTestsuites;

        OutputXmlAttribute(stream, kTestsuites, "tests",
            StreamableToString(unit_test.reportable_test_count()));
        OutputXmlAttribute(stream, kTestsuites, "failures",
            StreamableToString(unit_test.failed_test_count()));
        OutputXmlAttribute(
            stream, kTestsuites, "disabled",
            StreamableToString(unit_test.reportable_disabled_test_count()));
        OutputXmlAttribute(stream, kTestsuites, "errors", "0");
        OutputXmlAttribute(stream, kTestsuites, "time",
            FormatTimeInMillisAsSeconds(unit_test.elapsed_time()));
        OutputXmlAttribute(
            stream, kTestsuites, "timestamp",
            FormatEpochTimeInMillisAsIso8601(unit_test.start_timestamp()));

        if (GTEST_FLAG(shuffle)) {
            OutputXmlAttribute(stream, kTestsuites, "random_seed",
                StreamableToString(unit_test.random_seed()));
        }
        *stream << TestPropertiesAsXmlAttributes(unit_test.ad_hoc_test_result());

        OutputXmlAttribute(stream, kTestsuites, "name", "AllTests");
        *stream << ">\n";

        for (int i = 0; i < unit_test.total_test_suite_count(); ++i) {
            if (unit_test.GetTestSuite(i)->reportable_test_count() > 0)
                PrintXmlTestSuite(stream, *unit_test.GetTestSuite(i), i, capt_stdout);
        }
        *stream << "</" << kTestsuites << ">\n";
    }

    void EHReporter::PrintXmlTestsList(
        std::ostream* stream, const std::vector<TestSuite*>& test_suites, std::vector<std::vector<std::string>> capt_stdout) {
        const std::string kTestsuites = "testsuites";

        *stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        *stream << "<" << kTestsuites;

        int total_tests = 0;
        for (auto test_suite : test_suites) {
            total_tests += test_suite->total_test_count();
        }
        OutputXmlAttribute(stream, kTestsuites, "tests",
            StreamableToString(total_tests));
        OutputXmlAttribute(stream, kTestsuites, "name", "AllTests");
        *stream << ">\n";

        int counter = 0;
        for (auto test_suite : test_suites) {
            PrintXmlTestSuite(stream, *test_suite, counter++, capt_stdout);
        }
        *stream << "</" << kTestsuites << ">\n";
    }

    // Produces a string representing the test properties in a result as space
    // delimited XML attributes based on the property key="value" pairs.
    std::string EHReporter::TestPropertiesAsXmlAttributes(
        const TestResult& result) {
        Message attributes;
        for (int i = 0; i < result.test_property_count(); ++i) {
            const TestProperty& property = result.GetTestProperty(i);
            attributes << " " << property.key() << "="
                << "\"" << EscapeXmlAttribute(property.value()) << "\"";
        }
        return attributes.GetString();
    }

    void EHReporter::OutputXmlTestProperties(
        std::ostream* stream, const TestResult& result) {
        const std::string kProperties = "properties";
        const std::string kProperty = "property";

        if (result.test_property_count() <= 0) {
            return;
        }

        *stream << "<" << kProperties << ">\n";
        for (int i = 0; i < result.test_property_count(); ++i) {
            const TestProperty& property = result.GetTestProperty(i);
            *stream << "<" << kProperty;
            *stream << " name=\"" << EscapeXmlAttribute(property.key()) << "\"";
            *stream << " value=\"" << EscapeXmlAttribute(property.value()) << "\"";
            *stream << "/>\n";
        }
        *stream << "</" << kProperties << ">\n";
    }
} // namespace testing