#include "lexer.h"

#include <algorithm>
#include <charconv>
#include <unordered_map>
#include <iostream>
#include <streambuf>

using namespace std;

namespace parse {
    bool operator==(const Token &lhs, const Token &rhs) {
        using namespace token_type;

        if (lhs.index() != rhs.index()) {
            return false;
        }

        if (lhs.Is<Char>()) {
            return lhs.As<Char>().value == rhs.As<Char>().value;
        }

        if (lhs.Is<Number>()) {
            return lhs.As<Number>().value == rhs.As<Number>().value;
        }

        if (lhs.Is<String>()) {
            return lhs.As<String>().value == rhs.As<String>().value;
        }

        if (lhs.Is<Id>()) {
            return lhs.As<Id>().value == rhs.As<Id>().value;
        }

        return true;
    }

    bool operator!=(const Token &lhs, const Token &rhs) {
        return !(lhs == rhs);
    }

    std::ostream &operator<<(std::ostream &os, const Token &rhs) {
        using namespace token_type;

#define VALUED_OUTPUT(type) \
    if(auto  p = rhs.TryAs<type>()) return os << #type << '{' << p->value <<'}';
        VALUED_OUTPUT(Number);
        VALUED_OUTPUT(Id);
        VALUED_OUTPUT(String);
        VALUED_OUTPUT(Char);
#undef VALUED_OUTPUT

#define UNVALUED_OUTPUT(type) \
    if (rhs.Is<type>()) return os << #type;

        UNVALUED_OUTPUT(Class);
        UNVALUED_OUTPUT(Return);
        UNVALUED_OUTPUT(If);
        UNVALUED_OUTPUT(Else);
        UNVALUED_OUTPUT(Def);
        UNVALUED_OUTPUT(Newline);
        UNVALUED_OUTPUT(Print);
        UNVALUED_OUTPUT(Indent);
        UNVALUED_OUTPUT(Dedent);
        UNVALUED_OUTPUT(And);
        UNVALUED_OUTPUT(Or);
        UNVALUED_OUTPUT(Not);
        UNVALUED_OUTPUT(Eq);
        UNVALUED_OUTPUT(NotEq);
        UNVALUED_OUTPUT(LessOrEq);
        UNVALUED_OUTPUT(GreaterOrEq);
        UNVALUED_OUTPUT(None);
        UNVALUED_OUTPUT(True);
        UNVALUED_OUTPUT(False);
        UNVALUED_OUTPUT(Eof);

#undef UNVALUED_OUTPUT

        return os << "Unknown token :("sv;
    }

    Lexer::Lexer(std::istream &input) {
        CreateKeyWords();
        tokens_iterator_ = ParseTokens(input);
    }

    std::vector<Token>::const_iterator Lexer::ParseTokens(std::istream &input) {
        char current_char;
        size_t current_indent = 0;
        while (input.get(current_char)) {
            input.putback(current_char);
            ParseNumber(input);
            ParseString(input);
            ParseDoubleChar(input);
            ParseId(input);
            ParseChar(input);
            if (!tokens_.empty() && tokens_[tokens_.size()-1] == token_type::Newline{} )
                ParseIndent(input, current_indent, false);
            else
                ParseIndent(input, current_indent, true);
            ScipComments(input);

        }
        if(!tokens_.empty() && tokens_[tokens_.size() - 1] != token_type::Newline{})
            tokens_.emplace_back(token_type::Newline{});
        else
            tokens_.emplace_back(token_type::Eof{});
        for (size_t i = 0 / 2; i < current_indent / 2; i++)
            tokens_.emplace_back(token_type::Dedent{});
//        for (const auto &i: tokens_) {
//            cout << i << ' ';
//        }
//        cout << endl;

        return tokens_.begin();
    }

    void Lexer::ParseIndent(std::istream &input, size_t &current_indent, bool scip) {
        size_t count_spaces = 0;
        char current_char;
        input.get(current_char);
        while (current_char == ' ') {
            count_spaces += 1;
            input.get(current_char);
        }
        if (!scip) {
            for (size_t i = count_spaces / 2; i > current_indent / 2; i--)
                tokens_.emplace_back(token_type::Indent{});
            for (size_t i = count_spaces / 2; i < current_indent / 2; i++)
                tokens_.emplace_back(token_type::Dedent{});
            current_indent = count_spaces;
        }
        input.putback(current_char);
    }

    void Lexer::ScipComments(std::istream &input) {
        char current_char = (char) input.peek();

        if (current_char == '#') {
            string comment;
            getline(input, comment, '\n');
            input.putback('\n');
        }
    }


    void Lexer::ParseNumber(std::istream &input) {
        char current_char;
        current_char = (char) input.peek();

        if (isdigit(current_char)) {
            string string_number;
            input.get(current_char);

            while (isdigit(current_char)) {
                string_number += current_char;
                input.get(current_char);
                if (input.eof())
                    break;
            }
            input.putback(current_char);
            int number = stoi(string_number);
            tokens_.emplace_back(token_type::Number{number});
        }
    }

    void Lexer::ParseString(std::istream &input) {
        char current_char;
        current_char = (char) input.peek();
        char first = current_char;
        if (current_char == '\'' || current_char == '\"') {
            string str;
            input.get();
            input.get(current_char);
            while (current_char != first) {
                str += current_char;
                input.get(current_char);
            }
            tokens_.emplace_back(token_type::String{str});
        }
    }

    void Lexer::ParseId(std::istream &input) {
        char current_char;
        current_char = (char) input.peek();

        if (current_char == '_' || isalpha(current_char)) {
            input.get(current_char);
            string string_id;
            while (current_char == '_' || isalpha(current_char) || isdigit(current_char)) {
                string_id += current_char;
                input.get(current_char);
                if (input.eof())
                    break;
            }
            input.putback(current_char);
            if (keywords_.find(string_id) != keywords_.end())
                tokens_.emplace_back(keywords_.at(string_id));
            else
                tokens_.emplace_back(token_type::Id{string_id});
        }
    }

    void Lexer::ParseChar(std::istream &input) {
        char current_char, second_char;
        input.get(current_char);
        second_char = (char) input.peek();
        if (((current_char == '=' || current_char == '<' || current_char == '>') && second_char != '=')
            || current_char == ':' || current_char == '.' || current_char == ',' || current_char == '+'
            || current_char == '/' || current_char == '*' || current_char == '-'
            || current_char == '(' || current_char == ')')
            tokens_.emplace_back(token_type::Char{current_char});
        else if (current_char == '\n') {
            input.get(current_char);
            if(!tokens_.empty() && current_char != '\n' && !input.eof())
                tokens_.emplace_back(token_type::Newline{});
            input.putback(current_char);
        }
        else {
            input.putback(current_char);
        }

    }

    void Lexer::ParseDoubleChar(std::istream &input) {
        char first_char;
        input.get(first_char);
        char second_char = (char) input.peek();
        if (first_char == '<' && second_char == '=')
            tokens_.emplace_back(token_type::LessOrEq{});
        else if (first_char == '>' && second_char == '=')
            tokens_.emplace_back(token_type::GreaterOrEq{});
        else if (first_char == '=' && second_char == '=')
            tokens_.emplace_back(token_type::Eq{});
        else if (first_char == '!' && second_char == '=')
            tokens_.emplace_back(token_type::NotEq{});
        else {
            input.putback(first_char);
            return;
        }
        input.get(second_char);
    }

    const Token &Lexer::CurrentToken() const {
        return *tokens_iterator_;
    }

    Token Lexer::NextToken() {
        if (tokens_iterator_ + 1 == tokens_.end())
            return token_type::Eof{};
        return *(++tokens_iterator_);
    }

    void Lexer::CreateKeyWords() {
        keywords_["class"] = token_type::Class{};
        keywords_["return"] = token_type::Return{};
        keywords_["if"] = token_type::If{};
        keywords_["else"] = token_type::Else{};
        keywords_["def"] = token_type::Def{};
        keywords_["print"] = token_type::Print{};
        keywords_["and"] = token_type::And{};
        keywords_["or"] = token_type::Or{};
        keywords_["not"] = token_type::Not{};
        keywords_["None"] = token_type::None{};
        keywords_["True"] = token_type::True{};
        keywords_["False"] = token_type::False{};
    }
} // namespace parse