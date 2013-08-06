//  Created by Andrey Upadyshev aka Oliora.
//  This code was released to Public domain.

#ifndef batch_parser_batch_parser_h
#define batch_parser_batch_parser_h


#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/home/phoenix/container.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <iostream>
#include <string>


namespace batch_parser
{
    namespace detail
    {
        // TODO: CommandWithArgs must be a shared_ptr<std::vector<std::string> >
        // to reduce copy
        typedef std::vector<std::string> CommandWithArgs;
        //typedef std::vector<CommandWithArgs> CommandsList;
        
        
        struct Appender
        {
            template <typename C, typename Arg>
            struct result
            {
                typedef void type;
            };
            
            template <typename C, typename Arg>
            void operator()(C& c, Arg const& data) const
            {
                c.insert(c.end(), data.begin(), data.end());
            }
        };
        
        struct DebugPrinter
        {
            template <typename C>
            struct result
            {
                typedef void type;
            };

            template <typename C>
            void operator()(C arg) const
            {
                std::cout << arg << std::endl;
            }
        };
        
        /*void printComandsList(const CommandsList& cmds)
        {
            BOOST_FOREACH(const CommandWithArgs& cmd, cmds)
            {
                if (cmd.empty())
                {
                    std::cout << std::endl;
                    continue;
                }
                
                CommandWithArgs::const_iterator first = cmd.begin();
                std::cout << *first << ":" << std::endl;
                ++first;
                
                for(;first != cmd.end(); ++first)
                {
                    std::cout << "  " << *first << "," << std::endl;
                }
            }
        }*/
                
        void printCmd(const CommandWithArgs& cmd)
        {
            if (cmd.empty())
            {
                return;
            }
                
            CommandWithArgs::const_iterator first = cmd.begin();
            std::cout << *first << ":" << std::endl;
            ++first;
            
            for(;first != cmd.end(); ++first)
            {
                std::cout << "  " << *first << "," << std::endl;
            }
        }
                
        /*void printLabel(const std::string& label)
        {
            std::cout << ":" << label << std::endl;
        }

        void printComment(const std::string& comment)
        {
            std::cout << "// " << comment << std::endl;
        }*/
                
        typedef boost::spirit::ascii::space_type Skipper;
        
        boost::phoenix::function<Appender> const append;
        boost::phoenix::function<DebugPrinter> const dbg;


        struct BatchFileStatistic {
            BatchFileStatistic()
            : m_atMarks(0)
            , m_ifCommands(0)
            , m_forCommands(0)
            , m_comments(0)
            , m_labels(0)
            {}

            unsigned
                  m_atMarks
                , m_ifCommands
                , m_forCommands
                , m_comments
                , m_labels;
        };
        
        /*template <typename Iterator>
        struct BatchFileGrammar : boost::spirit::qi::grammar<Iterator, Skipper>
        {
            // TODO: move atMarkCount into high level attibute
            template<typename CommandAction>
            BatchFileGrammar(BatchFileStatistic& stat, CommandAction onCmd)
                : BatchFileGrammar::base_type(batch)
            {
                namespace qi = boost::spirit::qi;
                namespace ascii = boost::spirit::ascii;
                namespace phoenix = boost::phoenix;
                
                using qi::lit;
                using qi::lexeme;
                using qi::on_error;
                using qi::as_string;
                using qi::fail;
                using ascii::char_;
                using ascii::string;
                using ascii::blank;
                using ascii::space;
                using ascii::digit;
                using ascii::no_case;
                using namespace qi::labels;
                using boost::spirit::attr;
                using boost::spirit::eol;
                using boost::spirit::eoi;
                using boost::spirit::eps;
                using boost::spirit::hold;
                using boost::spirit::omit;
                using boost::spirit::raw;
                using boost::spirit::hold;
                using boost::spirit::no_skip;
                using boost::spirit::skip;
                using phoenix::push_back;
                using phoenix::ref;

                arg %= lexeme
                [
                    +(  (  char_ 
                         - (  space
                            | '&' | '|' | '\"' | '^' | '>' | '<' | '='
                            | (eps(ref(bracketsLevel) != 0) >> ')') // catch extra closing brackets
                            )
                         )
                      | (   lit('^')
                         >> -(  (   omit[eol] 
                                 >> -(
                                        (!eol >> char_)
                                      | (eol >> attr('\r') >> attr('\n'))
                                      )
                                 )
                              | (char_ - blank)
                              )
                         )
                      | raw[   char_('\"')
                            >> *(char_ - (char_('\"') | eol))
                            >> -char_('\"')
                            ] // I don't know why, but w/o raw[], the rule consumes some unprintable char on EOL/EOF
                      )
                 ];

                eqGarbage = omit[skip(blank)
                [
                    lit('=')
                 ]];

                at_mark = omit[skip(blank)
                [
                    lit('@')        [ref(stat.m_atMarks) += 1]
                 ]];

                argWithGarbage %= skip(blank)[*eqGarbage >> arg >> *eqGarbage];

                redirect = skip(blank)
                [
                       *eqGarbage
                    >> lexeme[     -digit
                                >> (  ">>" 
                                    | (lit('>') >> -lit('&'))
                                    | (lit('<') >> -lit('&'))
                                    )
                              ]
                    >> *eqGarbage
                    >> !char_(':') // to do not capture explicit labels
                    >> argWithGarbage
                 ];

                cmdCustom %= skip(blank)
                [
                       *omit[redirect]
                    >> (   *eqGarbage
                        >> !char_(':') // to do not capture explicit labels
                        >> argWithGarbage
                        )
                    >> *(omit[redirect] | argWithGarbage)
                 ];

                cmdIf = skip(blank)
                [
                       no_case["IF"]
                    >> *eqGarbage
                    >> -no_case["/I"]
                    >> *eqGarbage
                    >> -no_case["NOT"]
                    >> *eqGarbage
                    >> (  (
                              arg
                           >> no_case[lit("==") | "EQU" | "NEQ" | "LSS" | "LEQ" | "GTR" | "GEQ"]
                           >> arg
                           )
                        | (   no_case[lit("ERRORLEVEL") | "EXIST" | "CMDEXTVERSION" | "DEFINED"]
                           >> arg // TODO: 'arg' has different format depending on previous keyword
                           ) 
                        )
                    >> skip(space)[operand]
                    >> -(   no_case["ELSE"]
                         >> skip(space)[operand]
                         )
                 ];

                command = 
                       *(at_mark| eqGarbage)
                    >> (  cmdIf                 [ref(stat.m_ifCommands) += 1]
                        | cmdCustom             [onCmd]
                        )
                    >> *eqGarbage;

                label %= skip(blank)
                [
                       omit[   -(+omit[redirect] | (char_ - ':'))
                            >> *eqGarbage
                            >> ':'
                            >> *eqGarbage
                            ]
                    >> lexeme[
                              +(  (char_ - (space | '^'))
                                | (omit['^'] >> (char_ - (space | eol)))
                                | char_('^')
                                )
                        ]
                    >> omit[*(char_ - eol)]
                 ];

                comment %= skip(blank)
                [
                       *(at_mark| eqGarbage)
                    >> no_case["REM"]
                    >> lexeme[*(char_ - eol)]
                 ];
                
                group =
                       *(at_mark| eqGarbage | redirect| lexeme['^' >> eol])
                    >> char_('(')           [ref(bracketsLevel) += 1]
                    >> *expression
                    >> (char_(')') | eoi)   [ref(bracketsLevel) -= 1]
                    >> *(  eqGarbage
                         | redirect 
                         | lexeme['^' >> eol]
                         | (eps(ref(bracketsLevel) == 0) >> omit[+char_(')')]) // skip all extra closing brackets
                         );
                
                operation = (lit("&&") | "||" | '|' | '&');

                operand =
                      group
                    | comment           [ref(stat.m_comments) += 1]
                    | command
                    | label             [ref(stat.m_labels) += 1] // TODO: Match but don't report label if it's not at the begin of line
                    | +(at_mark | eqGarbage | redirect); // the rest mailformed shit

                expression =
                       operand
                    >> *(   operation
                         >> -expression // we are tolerant to errors as much as possible
                         );
                
                batch =
                       eps                  [ref(bracketsLevel) = 0]
                    >> *(expression);
            }
            
            long bracketsLevel; // TODO: move into attributes
            boost::spirit::qi::rule<Iterator, std::string()> arg;
            boost::spirit::qi::rule<Iterator, std::string()> argWithGarbage;
            boost::spirit::qi::rule<Iterator> eqGarbage;
            boost::spirit::qi::rule<Iterator> at_mark;
            boost::spirit::qi::rule<Iterator> redirect;
            boost::spirit::qi::rule<Iterator, CommandWithArgs()> cmdCustom;     // It have to has blank_type skipper, but compilation fails in such case. Why?
            boost::spirit::qi::rule<Iterator> cmdIf;                            // same problem
            boost::spirit::qi::rule<Iterator> command;                          // same problem
            boost::spirit::qi::rule<Iterator, std::string()> label;             // same problem
            boost::spirit::qi::rule<Iterator, std::string()> comment;           // same problem
            boost::spirit::qi::rule<Iterator, Skipper> expression;
            boost::spirit::qi::rule<Iterator, Skipper> group;
            boost::spirit::qi::rule<Iterator, Skipper> operand;
            boost::spirit::qi::rule<Iterator, Skipper> operation;
            boost::spirit::qi::rule<Iterator, Skipper> batch;
        };*/
        
        namespace lex = boost::spirit::lex;

        enum {
            IDANY
        };

        template <typename Lexer>
        struct BatchFileTokens : lex::lexer<Lexer>
        {
            BatchFileTokens()
            {
                this->self.add_pattern
                    ("NORMAL_WORD", "[^\s&|\"^><=]+");
                this->self.add_pattern
                    ("QUOTED_WORD", "\"[^\"\r\n]*(\")?");
                this->self.add_pattern
                    ("ESCAPED_CHAR", "\^\S")
                this->self.add_pattern
                    ("ARG", "({NORMAL_WORD} |  | (\^$) | {QUOTED_WORD})+"
                 
                word = "{ARG}";

                this->self.add
                    (word)
                    ('\n')
                    (".", IDANY)
                ;
            }

            lex::token_def<std::string> arg;
        };

        struct Printer
        {
            // the function operator gets called for each of the matched tokens
            // c, l, w are references to the counters used to keep track of the numbers
            template <typename Token>
            bool operator()(Token const& t) const
            {
                switch (t.id())
                {
                case ID_WORD:
                    break;
                case ID_EOL:
                    break;
                case ID_CHAR:
                    break;
                }
                return true;
            }
        };
    }
    
    template<typename Iterator>
    inline bool parse(Iterator first, Iterator last)
    {
        using namespace detail;

        typedef lex::lexertl::token<char const*, lex::omit, boost::mpl::false_> Token;
        typedef lex::lexertl::actor_lexer<Token> Lexer;

        BatchFileTokens<Lexer> lexer;

        char const* f = &*first;
        char const* l = &*last;
        bool r = lex::tokenize(f, l, tokenizer, Printer());

        std::cout << (r ? "OK" : "FAIL") << std::endl;

        /*namespace qi = boost::spirit::qi;
        namespace ascii = boost::spirit::ascii;
        namespace phoenix = boost::phoenix;
        
        detail::BatchFileStatistic stat;
        detail::BatchFileGrammar<Iterator> grammar(
            stat,
            detail::printCmd);
        
        
        bool r = phrase_parse(
                              first,
                              last,
                              grammar,
                              detail::Skipper()
                              );

        std::cout
            << "@: " << stat.m_atMarks << std::endl
            << "IFs: " << stat.m_ifCommands << std::endl
            << "FORs: " << stat.m_forCommands << std::endl
            << "Labels: " << stat.m_labels << std::endl
            << "Comments: " << stat.m_comments << std::endl
            << (r ? "OK" : "FAIL") << std::endl;
        
        if (first != last)
        {
            std::cerr << "Can't parse the following: " << std::endl << std::string(first, last) << std::endl;
            return false;
        }*/
        
        return true;
    }
}

#endif
