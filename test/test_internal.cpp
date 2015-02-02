#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include "yalibnkf.h"

struct TestResult
{
    bool ok;
    std::string what;
    std::vector<std::string> expected;
    std::string actual;
};

static std::string g_testing_putchar_buf;

extern "C" static void testing_putchar(int c)
{
    try
    {
        g_testing_putchar_buf += (unsigned char)c;
    }
    catch (...)
    {
    }
}

class TestCase
{
    std::string nkf_option_;
    std::string input_;
    std::vector<std::string> answers_;
    bool tolerant_;

public:
    TestCase(const char *nkf_option, const std::string &input, const std::vector<std::string> &answers)
      :
      nkf_option_(nkf_option),
      input_(input),
      answers_(answers),
      tolerant_(need_to_ignore_spaces(nkf_option))
    {
    }

    void add_answer(const std::string &answer)
    {
        answers_.emplace_back(answer);
    }

    TestResult run() const
    {
        std::vector<std::string> options { split(nkf_option_) };
        std::sort(options.begin(), options.end());

        do
        {
            std::string option { join(options) };

            yalibnkf_str result = yalibnkf_convert(option.c_str(), input_.c_str(), input_.size());
            std::string actual { result.str, result.len };
            yalibnkf_free(result);

            if (!matches_answer(actual))
            {
                return { false, {"yalibnkf_convert"}, answers_, actual };
            }
            
            g_testing_putchar_buf.clear();

            if (!yalibnkf_convert_fun(option.c_str(), input_.c_str(), input_.size(), testing_putchar))
            {
                return { false, {"yalibnkf_convert_fun failed"}, answers_, actual };
            }
            
            if (g_testing_putchar_buf != actual)
            {
                return { false, {"yalibnkf_convert_fun don't agree with yalibnkf_convert"}, answers_, actual };
            }
        }
        while (std::next_permutation(options.begin(), options.end()));

        return { true, {}, {}, {} };
    }

private:
    bool matches_answer(std::string actual) const
    {
        if (tolerant_)
        {
            strip(&actual);
        }

        return std::any_of(answers_.begin(), answers_.end(), [&](std::string answer)
        {
            if (tolerant_)
            {
                strip(&answer);
            }

            return actual == answer;
        });
    }

    /** !!($nkf_options =~ /-\w+m[NS]/) */
    static bool need_to_ignore_spaces(const char *nkf_options)
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

    static void strip(std::string *s)
    {
        s->erase(std::remove_if(s->begin(), s->end(), [](char c) { return c == ' '; }), s->end());
    }

    static std::vector<std::string> split(const std::string &s)
    {
        std::istringstream ss { s };
        std::vector<std::string> splat;
        std::string word;

        while (std::getline(ss, word, ' '))
        {
            if (!word.empty())
            {
                splat.push_back(std::move(word));
                word.clear();
            }
        }

        return splat;
    }

    static std::string join(const std::vector<std::string> &words)
    {
        std::ostringstream ss;

        for (auto i = words.begin(); i != words.end(); ++i)
        {
            if (i != words.begin())
            {
                ss << ' ';
            }

            ss << *i;
        }

        return ss.str();
    }
};

class TestBundle
{
    std::vector<std::string> description_;
    std::vector<TestCase> cases_;

public:
    const std::vector<std::string> &get_description() const
    {
        return description_;
    }

    void set_description(std::vector<std::string> &&desc)
    {
        description_ = desc;
    }

    void add_input(const char *nkf_option, std::string &&input)
    {
        cases_.emplace_back(TestCase { nkf_option, input, {} });
    }

    void add_answer(std::string &&answer)
    {
        ensure(!cases_.empty());
        cases_.back().add_answer(answer);
    }

    TestResult run() const
    {
        for (auto i = cases_.begin(); i != cases_.end(); ++i)
        {
            TestResult result = i->run();

            if (!result.ok)
            {
                return result;
            }
        }

        return {true, {}, {}, {}};
    }

private:
    void ensure(bool cond)
    {
        if (!cond)
        {
            throw std::runtime_error("unexpected state");
        }
    }
};

typedef std::vector<TestBundle> Test;

class TestLoader
{
    Test test_;
    std::vector<std::string> description_;

public:
    Test load()
    {
#include "test_cases.inc"
        ensure(description_.empty());
        return test_;
    }

private:
    void description(const char *message)
    {
        description_.emplace_back(message);
    }

    void test_input(const char *nkf_options, size_t input_length, const char *input)
    {
        if (!description_.empty())
        {
            test_.resize(test_.size() + 1);
            test_.back().set_description(std::move(description_));
            description_.clear();
        }

        ensure(!test_.empty());
        test_.back().add_input(nkf_options, std::string(input, input_length));
    }

    void test_output(size_t answer_length, const char *answer)
    {
        ensure(!test_.empty());
        test_.back().add_answer(std::string(answer, answer_length));
    }

    void ensure(bool cond)
    {
        if (!cond)
        {
            throw std::runtime_error("unexpected state");
        }
    }
};

class TestRunner
{
    int error_count_;
    Test test_;
    std::vector<std::string> labels_;

public:
    TestRunner(const Test &test, const std::vector<std::string> &labels)
      :
      error_count_(0),
      test_(test),
      labels_(labels)
    {
    }

    int error_count()
    {
        return error_count_;
    }

    void run()
    {
        std::for_each(test_.begin(), test_.end(), [=](const TestBundle &test)
        {
            if (is_selected(test))
            {
                run_case(test);
            }
        });
    }

private:
    bool is_selected(const TestBundle &test)
    {
        auto d = test.get_description();

        return labels_.empty() || std::any_of(labels_.begin(), labels_.end(), [&](const std::string &label)
        {
            return std::any_of(d.begin(), d.end(), [&](const std::string &desc)
            {
                return desc.find(label) != std::string::npos;
            });
        });
    }

    void run_case(const TestBundle &test)
    {
        print_description(test);

        TestResult result = test.run();

        if (result.ok)
        {
            puts("Ok");
        }
        else
        {
            printf("Fail\n  what: %s\n  expected: ", result.what.c_str());
            for (auto i = result.expected.begin(); i != result.expected.end(); ++i)
            {
                if (i != result.expected.begin())
                {
                    printf("\n  or: ");
                }
                dumpstr(*i);
            }
            fprintf(stdout, "\n  actual: ");
            dumpstr(result.actual);
            fprintf(stdout, "\n");

            error_count_++;
        }
    }

    void print_description(const TestBundle &test)
    {
        const auto &lines = test.get_description();

        for (auto i = lines.begin(); i != lines.end(); ++i)
        {
            if (i != lines.begin())
            {
                printf("\n");
            }
            printf("%-40s", i->c_str());
        }
    }

    static void dumpstr(const std::string &str)
    {
        putc('"', stdout);
        std::for_each(str.begin(), str.end(), [](char c)
        {
            fprintf(stdout, "\\%03o", (unsigned char)c);
        });
        putc('"', stdout);
    }
};

int main(int argc, char **argv)
{
    try
    {
        TestRunner test { TestLoader().load(), { &argv[1], &argv[argc] } };
        test.run();

        yalibnkf_quit();

        int error_count = test.error_count();

        if (error_count > 1)
        {
            printf("%d errors were found.\n", error_count);
        }
        else if (error_count == 1)
        {
            printf("1 error was found.\n");
        }
        else
        {
            printf("All tests are succeeded.\n");
            return 0;
        }
    }
    catch (const std::exception &e)
    {
        printf("Test has failed with exception: %s\n", e.what());
    }
    catch (...)
    {
        printf("Test has failed with some error.\n");
    }

    yalibnkf_quit();

    return 1;
}
