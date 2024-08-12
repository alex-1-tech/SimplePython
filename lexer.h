#pragma once

#include <iosfwd>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include <unordered_map>

namespace parse {
    namespace token_type {
        struct Number {
            int value;
        };// Лексема «число»
        struct Id {
            std::string value;
        };// Лексема «идентификатор»
        struct Char {
            char value;
        };// Лексема «символ»
        struct String {
            std::string value;
        };// Лексема «строковая константа»

        struct Class {
        };    // Лексема «class»
        struct Return {
        };   // Лексема «return»
        struct If {
        };       // Лексема «if»
        struct Else {
        };     // Лексема «else»
        struct Def {
        };      // Лексема «def»
        struct Newline {
        };  // Лексема «конец строки»
        struct Print {
        };    // Лексема «print»
        struct Indent {
        };  // Лексема «увеличение отступа», соответствует двум пробелам
        struct Dedent {
        };  // Лексема «уменьшение отступа»
        struct Eof {
        };     // Лексема «конец файла»
        struct And {
        };     // Лексема «and»
        struct Or {
        };      // Лексема «or»
        struct Not {
        };     // Лексема «not»
        struct Eq {
        };      // Лексема «==»
        struct NotEq {
        };   // Лексема «!=»
        struct LessOrEq {
        };     // Лексема «<=»
        struct GreaterOrEq {
        };  // Лексема «>=»
        struct None {
        };         // Лексема «None»
        struct True {
        };         // Лексема «True»
        struct False {
        };        // Лексема «False»
    } // namespace token_type

    using TokenBase = std::variant<token_type::Number, token_type::Id, token_type::Char, token_type::String,
            token_type::Class, token_type::Return, token_type::If, token_type::Else,
            token_type::Def, token_type::Newline, token_type::Print, token_type::Indent,
            token_type::Dedent, token_type::And, token_type::Or, token_type::Not,
            token_type::Eq, token_type::NotEq, token_type::LessOrEq, token_type::GreaterOrEq,
            token_type::None, token_type::True, token_type::False, token_type::Eof>;

    struct Token : TokenBase {
        using TokenBase::TokenBase;

        template<typename T>
        [[nodiscard]] bool Is() const {
            // Возвращается true, если существует вариант.
            return std::holds_alternative<T>(*this);
        }

        template<typename T>
        [[nodiscard]] const T &As() const {
            return std::get<T>(*this);
        }

        template<typename T>
        [[nodiscard]] const T *TryAs() const {
            return std::get_if<T>(this);
        }
    };

    bool operator==(const Token &lhs, const Token &rhs);

    bool operator!=(const Token &lhs, const Token &rhs);

    std::ostream &operator<<(std::ostream &os, const Token &rhs);

    class LexerError : public std::runtime_error {
    public:
        using std::runtime_error::runtime_error;
    };

    class Lexer {
    public:
        explicit Lexer(std::istream &input);

        // Возвращает ссылку на текущий токен или token_type::Eof, если поток токенов закончился
        [[nodiscard]] const Token &CurrentToken() const;

        // Возвращает следующий токен, либо token_type::Eof, если поток токенов закончился
        Token NextToken();

        template<typename T>
        const T &Expect() const {
            using namespace std::literals;
            if(CurrentToken().Is<T>())
                return CurrentToken().As<T>();
            throw LexerError("Not implemented"s);
        }

        template<typename T, typename U>
        void Expect(const U &value) const {
            using namespace std::literals;
            if(Expect<T>().value != value)
                throw LexerError("Not implemented"s);
        }

        template <typename T>
        const T& ExpectNext() {
            using namespace std::literals;
            NextToken();
            return Expect<T>();
        }

        template <typename T, typename U>
        void ExpectNext(const U& value) {
            using namespace std::literals;
            NextToken();
            Expect<T>(value);
        }

    private:
        std::vector<Token>::const_iterator ParseTokens(std::istream &input);

        void CreateKeyWords();

        void ParseNumber(std::istream &);

        void ParseString(std::istream &);

        void ParseId(std::istream &);

        void ParseChar(std::istream &);

        void ParseDoubleChar(std::istream &);

        void ParseIndent(std::istream &, size_t &, bool);

        void ScipComments(std::istream &);
    private:
        std::vector<Token> tokens_;
        std::vector<Token>::const_iterator tokens_iterator_;
        std::unordered_map<std::string, Token> keywords_;


    };

} // namespace parse