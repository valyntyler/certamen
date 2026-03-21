#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <fstream>
#include <limits>
#include <iomanip>
#include <random>
#include <cctype>
#include <algorithm>
#include <yaml-cpp/yaml.h>

struct Question
{
    std::string question;
    std::vector<std::string> choices;
    int answer; // 'answer' is an index onto 'choices'.
    std::optional<std::string> code;
    std::optional<std::string> explain;
};

// ensures 'choices' > 1, answer is present in the range of choices, and question is non-empty.
static std::optional<std::string> validate_question(const Question& q)
{
    if (q.question.empty()) return std::optional<std::string>("Question cannot be blank: ");
    if (q.choices.size() < 2) return std::optional<std::string>("Question must have more than one (1) choice: ");
    if (q.answer < 0 || q.answer >= static_cast<int>(q.choices.size()))
        return std::optional<std::string>("Answer provided is out of range of the number of choices provided. Re-enter.");
    return std::nullopt;
}

static int read_int_in_range(int min_val, int max_val, const std::string& prompt)
{
    while (true)
    {
        std::cout << prompt;
        int value;
        if (std::cin >> value)
        {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (value >= min_val && value <= max_val) return value;
            std::cout << "Invalid. Enter a number between " << min_val << " and " << max_val << ".\n";
        } else {
            std::cout << "Invalid. Enter a number (integer) within bounds of the question (i.e. between "<< min_val << " and " << max_val << ").\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }
}

static std::string read_line(const std::string& prompt)
{
    while (true)
    {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);
        if (!s.empty()) return s;
        std::cout << "Invalid. Input must be non-empty. Re-enter.\n";
    }
}

static bool read_yes_no(const std::string& prompt)
{
    while (true)
    {
        std::cout << prompt << " (y/n): ";
        std::string s;
        std::getline(std::cin, s);
        if (!s.empty())
        {
            char c = static_cast<char>(std::tolower(s[0]));
            if (c == 'y') return true;
            if (c == 'n') return false;
        }
        std::cout << "Invalid. Answer with 'y' (as in yes) or 'n' (as in no).\n";
    }
}
// accumulate lines until encountering the terminator string.
static std::string read_multiline_until(const std::string& header, const std::string& terminator)
{
    std::cout << header << "\n";
    std::cout << "(Finish by entering a line with only " << terminator << ")\n";
    std::string out, line;
    while (true)
    {
        std::getline(std::cin, line);
        if (line == terminator) break;
        out.append(line);
        out.push_back('\n');
    }
    if (!out.empty() && out.back() == '\n') out.pop_back();
    return out;
}

static std::vector<Question> load_questions(const std::string& filename) {
    YAML::Node root = YAML::LoadFile(filename);
    if (!root.IsSequence())
    {
        throw std::runtime_error("YAML must be a sequence, to fix this, look at the example YAML (compare with your .yaml) and ensure your file starts with a dash (-) for each question.");
    }

    std::vector<Question> qs;
    qs.reserve(root.size());

    for (const auto& item : root) {
        if (!item.IsMap()) throw std::runtime_error("Each entry must be a map, to fix this, look at the example YAML (compare with your .yaml) and ensure each question entry is a map (i.e. starts with a dash (-) and then has keys like 'question', 'choices', etc. indented under it).");

        const auto qNode    = item["question"];
        const auto cNode    = item["choices"];
        const auto aNode    = item["answer"];
        const auto codeNode = item["code"];
        const auto eNode    = item["explain"];

        if (!qNode || !cNode || !aNode) throw std::runtime_error("Missing required keys (question, choices, answer) to fix this, find any given question in your .yaml file which is missing these attributes.");
        if (!cNode.IsSequence()) throw std::runtime_error("'choices' must be a sequence, to fix this, look at the example YAML (compare with your .yaml) and ensure 'choices' is a list (i.e. starts with a dash (-) for each choice and is indented under the question).");

        Question q;
        q.question = qNode.as<std::string>();
        q.choices.reserve(cNode.size());
        for (const auto& c : cNode) q.choices.push_back(c.as<std::string>());
        q.answer = aNode.as<int>();
        if (codeNode) q.code = codeNode.as<std::string>();
        if (eNode)    q.explain = eNode.as<std::string>();

        if (auto err = validate_question(q)) throw std::runtime_error("Invalid question: " + *err);
        qs.push_back(std::move(q));
    }

    return qs;
}

// emit a sequence of maps, write 'code' and 'explain' as literal blocks, save. WARN: I think this can be better
static void save_questions(const std::vector<Question>& qs, const std::string& filename)
{
    YAML::Emitter out;
    out << YAML::BeginSeq;
    for (const auto& q : qs)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "question" << YAML::Value << q.question;
        if (q.code && !q.code->empty())
        {
            out << YAML::Key << "code" << YAML::Value << YAML::Literal << *q.code;
        }
        if (q.explain && !q.explain->empty()) {
            out << YAML::Key << "explain" << YAML::Value << YAML::Literal << *q.explain;
        }
        out << YAML::Key << "choices" << YAML::Value << YAML::BeginSeq;
        for (const auto& c : q.choices) out << c;
        out << YAML::EndSeq;
        out << YAML::Key << "answer" << YAML::Value << q.answer;
        out << YAML::EndMap;
    }
    out << YAML::EndSeq;

    std::ofstream os(filename);
    if (!os) throw std::runtime_error("Cannot open file for writing, check permissions given to the program and disk space (and check if its not open in another editor e.g. Notepad): " + filename);
    os << out.c_str();
}

static void print_question(const Question& q, std::size_t idx, std::size_t total)
{
    std::cout << "Question " << (idx + 1) << " of " << total << ":\n";
    std::cout << q.question << "\n";
    if (q.code && !q.code->empty())
    {
        std::cout << "\n--- Code ---\n";
        std::cout << *q.code << "\n";
        std::cout << "------------\n";
    }
    for (std::size_t i = 0; i < q.choices.size(); ++i)
    {
        std::cout << "  " << (i + 1) << ". " << q.choices[i] << "\n";
    }
}

// shuffle the choices of any given question and adjust its answer index. WARN: I want to change this to use /dev/urandom.
static Question shuffle_question_choices(const Question& q, std::mt19937& rng)
{
    std::vector<std::pair<std::string, bool>> entries;
    entries.reserve(q.choices.size());
    for (std::size_t i = 0; i < q.choices.size(); ++i)
    {
        entries.emplace_back(q.choices[i], static_cast<int>(i) == q.answer);
    }
    std::shuffle(entries.begin(), entries.end(), rng);

    Question out;
    out.question = q.question;
    out.code     = q.code;
    out.explain  = q.explain;
    out.choices.reserve(entries.size());
    out.answer = 0;

    for (std::size_t i = 0; i < entries.size(); ++i)
    {
        out.choices.push_back(entries[i].first);
        if (entries[i].second) out.answer = static_cast<int>(i);
    }
    return out;
}

// map each question through shuffle_question_choices, then shuffle the vector of questions.
static std::vector<Question> randomize_quiz(const std::vector<Question>& qs, std::mt19937& rng) {
    std::vector<Question> shuffled;
    shuffled.reserve(qs.size());
    for (const auto& q : qs) {
        shuffled.push_back(shuffle_question_choices(q, rng));
    }
    std::shuffle(shuffled.begin(), shuffled.end(), rng);
    return shuffled;
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

static void take_quiz(const std::vector<Question>& qs, bool randomise)
{
    if (qs.empty())
    {
        std::cout << "No questions available. Add some first.\n";
        return;
    }

    std::random_device rd;
    std::mt19937 rng(rd());
    const std::vector<Question> session_qs = randomise ? randomize_quiz(qs, rng) : qs;

    int correct = 0;
    std::cout << "\nStarting Quiz " << (randomise ? "(randomised)" : "") << " \n";
    for (std::size_t i = 0; i < session_qs.size(); ++i) {
        print_question(session_qs[i], i, session_qs.size());
        int ans = read_int_in_range(1, static_cast<int>(session_qs[i].choices.size()), "Your answer: ");
        if ((ans - 1) == session_qs[i].answer)
        {
            std::cout << "Correct!\n";
            ++correct;
            
            std::cout << "Score: " << correct << "/" << session_qs.size();
            double pct = session_qs.empty() ? 0.0 : (100.0 * static_cast<double>(correct) / static_cast<double>(session_qs.size()));
            std::cout << " (" << std::fixed << std::setprecision(1) << pct << "%)\n";
        } else
        {
            std::cout << "Incorrect. The correct answer is " << (session_qs[i].answer + 1)
                      << ". " << session_qs[i].choices[session_qs[i].answer] << "\n";
        }
        if (session_qs[i].explain && !session_qs[i].explain->empty()) {
            std::cout << "Explanation:\n" << *session_qs[i].explain << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "Quiz Complete.\n";
    std::cout << "Score: " << correct << "/" << session_qs.size();
    double pct = session_qs.empty() ? 0.0 : (100.0 * static_cast<double>(correct) / static_cast<double>(session_qs.size()));
    std::cout << " (" << std::fixed << std::setprecision(1) << pct << "%)\n";
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

static std::vector<Question> add_question(const std::vector<Question>& qs)
{
    std::vector<Question> out = qs;

    std::string qtext = read_line("\nEnter the question: ");

    std::optional<std::string> code = std::nullopt;
    if (read_yes_no("Include a code snippet?"))
    {
        code = read_multiline_until("Paste your code below", "END");
    }

    std::optional<std::string> explain = std::nullopt;
    if (read_yes_no("Include an explanation for the correct answer?"))
    {
        explain = read_multiline_until("Enter the explanation", "END");
    }

    int n = read_int_in_range(2, 10, "How many choices (2-10): ");
    std::vector<std::string> choices;
    choices.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i)
    {
        choices.push_back(read_line("Choice " + std::to_string(i + 1) + ": "));
    }

    int correct = read_int_in_range(1, n, "Which choice is correct (1-" + std::to_string(n) + "): ");

    Question q;
    q.question = qtext;
    q.choices  = std::move(choices);
    q.answer   = correct - 1;
    q.code     = std::move(code);
    q.explain  = std::move(explain);

    if (auto err = validate_question(q))
    {
        std::cout << "Error: " << *err << "\n";
        return out;
    }

    out.push_back(std::move(q));
    std::cout << "Question added.\n";
    return out;
}

static std::vector<Question> remove_question(const std::vector<Question>& qs)
{
    if (qs.empty())
    {
        std::cout << "No questions to remove.\n";
        return qs;
    }
    std::cout << "\nRemove Question\n";
    for (std::size_t i = 0; i < qs.size(); ++i) {
        std::cout << (i + 1) << ". " << qs[i].question
                  << (qs[i].code ? " [code]" : "")
                  << (qs[i].explain ? " [explain]" : "") << "\n";
    }
    int idx = read_int_in_range(1, static_cast<int>(qs.size()), "Remove which (1-" + std::to_string(qs.size()) + "): ");

    std::vector<Question> out;
    out.reserve(qs.size() - 1);
    for (std::size_t i = 0; i < qs.size(); ++i)
    {
        if (static_cast<int>(i) != (idx - 1)) out.push_back(qs[i]);
    }
    std::cout << "Question removed.\n";
    return out;
}

// display the current question, then set the new answer index in a copy.
static std::vector<Question> change_answer(const std::vector<Question>& qs)
{
    if (qs.empty())
    {
        std::cout << "No questions available.\n";
        return qs;
    }
    std::cout << "\nChange Answer\n";
    for (std::size_t i = 0; i < qs.size(); ++i)
    {
        std::cout << (i + 1) << ". " << qs[i].question << "\n";
    }
    int idx = read_int_in_range(1, static_cast<int>(qs.size()), "Modify which (1-" + std::to_string(qs.size()) + "): ");

    const Question& chosen = qs[static_cast<std::size_t>(idx - 1)];
    print_question(chosen, static_cast<std::size_t>(idx - 1), qs.size());
    std::cout << "Current correct answer: " << (chosen.answer + 1) << ". " << chosen.choices[chosen.answer] << "\n";

    int new_ans = read_int_in_range(1, static_cast<int>(chosen.choices.size()), "New correct answer: ");

    std::vector<Question> out = qs;
    out[static_cast<std::size_t>(idx - 1)].answer = new_ans - 1;
    std::cout << "Answer updated!\n";
    return out;
}

static void list_questions(const std::vector<Question>& qs)
{
    if (qs.empty())
    {
        std::cout << "No questions available.\n";
        return;
    }
    std::cout << "\nAll Questions::\n";
    for (std::size_t i = 0; i < qs.size(); ++i)
    {
        std::cout << (i + 1) << ". " << qs[i].question
                  << (qs[i].code ? " [code]" : "")
                  << (qs[i].explain ? " [explain]" : "") << "\n";
    }
}

static int menu_choice(bool randomise)
{
    std::cout << "\nQuizzer (Randomise: " << (randomise ? "ON" : "OFF") << ")\n"
                 "1. Take Quiz\n"
                 "2. Add Question\n"
                 "3. Remove Question\n"
                 "4. Change Answer\n"
                 "5. List Questions\n"
                 "6. Save and Exit\n"
                 "7. Toggle Randomise\n";
    return read_int_in_range(1, 7, "Choose (1-7): ");
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

int main(int argc, char* argv[]) {
    try
    {
        const std::string filename = (argc > 1) ? std::string(argv[1]) : std::string("quiz.yaml");

        std::vector<Question> questions;
        try
        {
            questions = load_questions(filename);
            std::cout << "Loaded " << questions.size() << " questions from " << filename << ".\n";
        } catch (const std::exception& e)
        {
            std::cout << "Warning: " << e.what() << "\nThus, Starting with an empty quiz.\n";
        }

        bool randomise = false;

        while (true)
        {
            int choice = menu_choice(randomise);
            switch (choice) {
                case 1:
                    take_quiz(questions, randomise);
                    break;
                case 2:
                    questions = add_question(questions);
                    break;
                case 3:
                    questions = remove_question(questions);
                    break;
                case 4:
                    questions = change_answer(questions);
                    break;
                case 5:
                    list_questions(questions);
                    break;
                case 6:
                    try {
                        save_questions(questions, filename);
                        std::cout << "Saved to " << filename << ". =)\n";
                    } catch (const std::exception& e) {
                        std::cerr << "Save error: " << e.what() << "\n";
                        std::cerr << "Changes are UN-saved! =(\n";
                        return 1;
                    }
                    return 0;
                case 7:
                    randomise = !randomise;
                    std::cout << "Randomise is now " << (randomise ? "ON" : "OFF") << ".\n";
                    break;
            }
        }
    } catch (const std::bad_alloc&)
    {
        std::cerr << "Fatal: Out of memory.\n";
        return 1;
    } catch (const std::exception& e)
    {
        std::cerr << "Fatal error: " << e.what() << ". Please report this as a github issue. (see README.md)\n";
        return 1;
    } catch (...)
    {
        std::cerr << "Fatal: Unknown error. Please report this as a github issue (see README.md)\n";
        return 1;
    }
}
