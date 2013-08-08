//  Created by Andrey Upadyshev aka Oliora.
//  This code was released to Public domain.

#ifndef batch_parser_batch_parser_h
#define batch_parser_batch_parser_h


#include <boost/spirit/include/qi.hpp>
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
    // Command and its arguments
    typedef std::vector<std::string> Command;

    struct Statistic {
        Statistic()
        : m_atMarks(0)
        , m_ifCommands(0)
        , m_forCommands(0)
        , m_comments(0)
        , m_labels(0)
        , m_redirects(0)
        {}

        unsigned
              m_atMarks
            , m_ifCommands
            , m_forCommands
            , m_comments
            , m_labels
            , m_redirects;
    };

    namespace ns = boost::spirit::standard;
        
    template <typename Iterator>
    struct Grammar : boost::spirit::qi::grammar<Iterator, ns::space_type>
    {
        // TODO: move atMarkCount into high level attibute
        template<typename CommandAction>
        Grammar(Statistic& stat, CommandAction onCmd)
            : Grammar::base_type(batch)
        {
            namespace qi = boost::spirit::qi;
            namespace phoenix = boost::phoenix;
            using namespace qi::labels;

            using qi::lit;
            using qi::lexeme;
            using qi::on_error;
            using qi::as_string;
            using qi::fail;
            
            using ns::char_;
            using ns::blank;
            using ns::space;
            using ns::alpha;
            using ns::digit;
            using ns::no_case;
            
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
                             >> -(  (omit[eol] >> attr('\n'))
                                  | (char_ - blank)
                                  )
                             )
                          | (char_ - blank)
                          )
                     )
                  | (   char_('\"')
                     >> *(char_ - char_('\"') - eol)
                     >> -char_('\"')
                     )
                  )
             ];

            eqGarbage = omit[skip(blank)
            [
                +lit('=')
             ]];

            at_mark = omit[skip(blank)
            [
                +(lit('@')                  [ref(stat.m_atMarks) += 1])
             ]];

            argWithGarbage %= skip(blank)[*eqGarbage >> arg >> *eqGarbage];

            redirect = skip(blank)
            [(
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
             )                              
            ][ref(stat.m_redirects) += 1];

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
                >> -lexeme[char_('/') >> alpha]
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

            cmdFor = skip(blank)
            [
                   no_case["FOR"]
                >> *eqGarbage
                >> -lexeme[lit('/') >> alpha]
                >> *eqGarbage
                >> -(!lit('%') >> argWithGarbage)
                /*>> -(drive)*/
                /*>> -(options)*/
                >> lexeme[lit("%%") >> alpha]
                >> *eqGarbage
                >> no_case["IN"]
                >> *eqGarbage
                >> lit('(')                 [ref(bracketsLevel) += 1]
                >> skip(space)
                   [
                       *argWithGarbage
                    >> lit(')')             [ref(bracketsLevel) -= 1]
                    ]
                >> *eqGarbage
                >> no_case["DO"]
                >> skip(space)[operand]
             ];

            command = 
                   *(at_mark| eqGarbage)
                >> (  cmdIf                 [ref(stat.m_ifCommands) += 1]
                    | cmdFor                [ref(stat.m_forCommands) += 1]
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
                >> char_('(')               [ref(bracketsLevel) += 1]
                >> *expression
                >> (char_(')') | eoi)       [ref(bracketsLevel) -= 1]
                >> *(  eqGarbage
                     | redirect 
                     | lexeme['^' >> eol]
                     | (eps(ref(bracketsLevel) == 0) >> omit[+char_(')')]) // skip all extra closing brackets
                     );
            
            operation = (lit("&&") | "||" | '|' | '&');

            operand =
                  group
                | comment                   [ref(stat.m_comments) += 1]
                | command
                | label                     [ref(stat.m_labels) += 1] // TODO: Match but don't report label if it's not at the begin of line
                | +(at_mark | eqGarbage | redirect); // the rest mailformed shit

            expression =
                   operand
                >> *(   operation
                     >> -expression // we are tolerant to errors as much as possible
                     );
            
            batch =
                   eps                      [ref(bracketsLevel) = 0]
                >> *(expression);
        }
        
        long bracketsLevel; // TODO: move into attributes
        boost::spirit::qi::rule<Iterator, std::string()> arg;
        boost::spirit::qi::rule<Iterator, std::string()> argWithGarbage;
        boost::spirit::qi::rule<Iterator> eqGarbage;
        boost::spirit::qi::rule<Iterator> at_mark;
        boost::spirit::qi::rule<Iterator> redirect;
        boost::spirit::qi::rule<Iterator, Command()> cmdCustom;             // It have to has blank_type skipper, but compilation fails in such case. Why?
        boost::spirit::qi::rule<Iterator> cmdIf;                            // same problem
        boost::spirit::qi::rule<Iterator> cmdFor;                           // same problem
        boost::spirit::qi::rule<Iterator> command;                          // same problem
        boost::spirit::qi::rule<Iterator, std::string()> label;             // same problem
        boost::spirit::qi::rule<Iterator, std::string()> comment;           // same problem
        boost::spirit::qi::rule<Iterator, ns::space_type> expression;
        boost::spirit::qi::rule<Iterator, ns::space_type> group;
        boost::spirit::qi::rule<Iterator, ns::space_type> operand;
        boost::spirit::qi::rule<Iterator, ns::space_type> operation;
        boost::spirit::qi::rule<Iterator, ns::space_type> batch;
    };
    
    template<typename Iterator, typename CommandAction>
    inline bool parse(Iterator& first, Iterator last, Statistic& statistic, CommandAction onCmd)
    {
        namespace qi = boost::spirit::qi;

        Grammar<Iterator> grammar(statistic, onCmd);
        
        return qi::phrase_parse(first, last, grammar, ns::space);
    }

        /*
            << (r ? "OK" : "FAIL") << std::endl;
        
        if (first != last)
        {
            std::cerr << "Can't parse the following: " << std::endl << std::string(first, last) << std::endl;
            return false;
        }
        
        return true;
    }*/
}

#endif
