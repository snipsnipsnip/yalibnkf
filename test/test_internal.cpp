#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <utility>
#include <algorithm>
#include "yalibnkf.h"

class Test
{
    int case_count_;
    int error_count_;
    int last_case_count_;
    int last_error_count_;

public:
    Test()
      :
      case_count_(0),
      error_count_(0),
      last_case_count_(0),
      last_error_count_(0)
    {
    }

    int error_count()
    {
        return error_count_;
    }

    void run()
    {
#include "test_cases.inc"
        report_result();
    }

private:
    void description(const char *message)
    {
        report_result();
        printf("%-40s", message);
    }
    
    void report_result()
    {
        if (case_count_ == last_case_count_)
        {
            printf("\n");
        }
        else if (last_error_count_ == error_count_)
        {
            puts("Ok");
        }
        
        last_case_count_ = case_count_;
        last_error_count_ = error_count_;
    }

    typedef std::pair<size_t, const char *> Answer;
    typedef std::vector<Answer> Answers;

    void testcase(const char *nkf_options,
                  size_t input_length, const char *input,
                  size_t answer_length, const char *answer,
                  int num_alternative_answers, ...)
    {
        va_list ap;
        Answers answers(num_alternative_answers + 1);

        answers[0] = Answer(answer_length, answer);

        va_start(ap, num_alternative_answers);
        for (int i = 0; i < num_alternative_answers; i++)
        {
            answers[i + 1].first = va_arg(ap, size_t);
            answers[i + 1].second = va_arg(ap, const char *);
        }
        va_end(ap);

        yalibnkf_result_t result = yalibnkf_convert(input, input_length, nkf_options);

        if (need_to_ignore_spaces(nkf_options))
        {
            assert_nearly_equal(answers, result);
        }
        else
        {
            assert_equal(answers, result);
        }

        yalibnkf_free(result);
        case_count_++;
    }

    void assert_equal(const Answers &answers, const yalibnkf_result_t &actual)
    {
        for (Answers::const_iterator i = answers.begin(); i != answers.end(); ++i)
        {
            const Answer &expected = *i;

            if (actual.len == expected.first && memcmp(actual.str, expected.second, actual.len) == 0)
            {
                return;
            }
        }

        fprintf(stdout, "Fail\n  expected: ");
        dumpstr(answers[0].first, answers[0].second);
        fprintf(stdout, "\n  actual: ");
        dumpstr(actual.len, actual.str);
        fprintf(stdout, "\n");

        error_count_++;
    }
    
    void dumpstr(size_t len, const char *str)
    {
        putc('"', stdout);
        for (size_t i = 0; i < len; i++)
        {
            fprintf(stdout, "\\%03o", (unsigned char)str[i]);
        }
        putc('"', stdout);
    }

    /** !!($nkf_options =~ /-\w+m[NS]/) */
    bool need_to_ignore_spaces(const char *nkf_options)
    {
        for (const char *p = nkf_options; (p = strchr(p, '-')) != NULL;)
        {
            p++;
            for (; isalnum(*p); p++)
            {
                if (*p == 'm' && (p[1] == 'N' || p[1] == 'S'))
                {
                    return true;
                }
            }
        }

        return false;
    }

    /** Compares result ignoring spaces. */
    void assert_nearly_equal(const Answers &answers, const yalibnkf_result_t &actual)
    {
        std::vector<char> stripped_actual = strip(actual.str, actual.len);

        for (Answers::const_iterator i = answers.begin(); i != answers.end(); ++i)
        {
            std::vector<char> stripped_expected = strip(i->second, i->first);

            if (stripped_actual == stripped_expected)
            {
                return;
            }
        }

        error_count_++;
    }

    std::vector<char> strip(const char *str, size_t len)
    {
        std::vector<char> stripped(str, str + len);

        stripped.erase(std::remove_if(stripped.begin(), stripped.end(), is_space), stripped.end());

        return stripped;
    }

    static bool is_space(char c)
    {
        return c == ' ';
    }
};

int main(int argc, char **argv)
{
    Test test;
    test.run();

    int error_count = test.error_count();

    if (error_count > 1)
    {
        printf("%d errors were found.\n", error_count);
        return 1;
    }
    else if (error_count == 1)
    {
        printf("1 error was found.\n");
        return 1;
    }
    else
    {
        printf("All tests are succeeded.\n");
        return 0;
    }
}
