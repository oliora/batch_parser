//
//  parser2.h
//  batch_parser
//
//  Created by Andrey Upadyshev on 15.07.13.
//  Copyright (c) 2013 Andrey Upadyshev. All rights reserved.
//

#ifndef batch_parser_parser2_h
#define batch_parser_parser2_h

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
    namespace detail
    {
        // TODO: CommandWithArgs must be a shared_ptr<std::vector<std::string> >
        // to reduce copy
        typedef std::vector<std::string> CommandWithArgs;
        typedef std::vector<CommandWithArgs> CommandsList;
        
        
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
        
        void printComandsList(const CommandsList& cmds)
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
        }
        
        typedef boost::spirit::ascii::space_type Skipper;
        
        
        template <typename Iterator>
        struct BatchFileGrammar : boost::spirit::qi::grammar<Iterator, CommandsList(), Skipper>
        {
            BatchFileGrammar(size_t& atMarkCount) : BatchFileGrammar::base_type(batch)
            {
                namespace qi = boost::spirit::qi;
                namespace ascii = boost::spirit::ascii;
                namespace phoenix = boost::phoenix;
                
                using qi::lit;
                using qi::lexeme;
                using qi::on_error;
                using qi::fail;
                using ascii::char_;
                using ascii::string;
                using ascii::blank;
                using ascii::space;
                using namespace qi::labels;
                using boost::spirit::eol;
                using boost::spirit::eoi;
                using boost::spirit::hold;
                using boost::spirit::no_skip;
                
                
                boost::phoenix::function<Appender> const append;
                
                arg =
                    lexeme[+char_("a-zA-Z0-9_-")[_val += _1]];
                
                command = (arg[phoenix::push_back(_val, _1)] % +blank);// - (eol | eoi | "&&" | "||" | '&' | '|');
                
                group =
                    char_('(') >> expression[append(_val, _1)] >> char_(')');
                
                operation =
                    (command[phoenix::push_back(_val, _1)] | group[append(_val, _1)]) >>
                        no_skip[*blank >> (eol | ((lit("&&") | "||"  | '|') >> *blank >> !eol) | char_('&'))] >>
                        expression[append(_val, _1)];
                
                // TODO: fix dogs counter (why it's too big number?)
                expression = *char_('@')[phoenix::ref(atMarkCount) += 1] >> (
                    operation[append(_val, _1)] |
                    command[phoenix::push_back(_val, _1)] |
                    group[append(_val, _1)]);
                
                
                batch = *(expression[append(_val, _1)] | +space) >>
                    *char_(')'); // #crazy_bat: any amount of extra closing brackets are allowed at the end of file
                
//                on_error<fail>
//                (
//                    batch
//                    , std::cout
//                        << phoenix::val("Error! Expecting ")
//                        << _4                               // what failed?
//                        << phoenix::val(" here: \"")
//                        << phoenix::construct<std::string>(_3, _2)   // iterators to error-pos, end
//                        << phoenix::val("\"")
//                        << std::endl
//                );
                
//                BOOST_SPIRIT_DEBUG_NODE(batch);
            }
            
            boost::spirit::qi::rule<Iterator, std::string()> arg;
            boost::spirit::qi::rule<Iterator, CommandWithArgs()> command;
            boost::spirit::qi::rule<Iterator, CommandsList(), Skipper> expression;
            boost::spirit::qi::rule<Iterator, CommandsList(), Skipper> group;
            boost::spirit::qi::rule<Iterator, CommandsList(), Skipper> operation;
            boost::spirit::qi::rule<Iterator, CommandsList(), Skipper> batch;
        };
        
    }
    
    template<typename Iterator>
    inline bool parse(Iterator first, Iterator last)
    {
        namespace qi = boost::spirit::qi;
        namespace ascii = boost::spirit::ascii;
        namespace phoenix = boost::phoenix;
        
        detail::CommandsList commands;
        size_t atMarkCount = 0;
        bool r = phrase_parse(
                              first,
                              last,
                              detail::BatchFileGrammar<Iterator>(atMarkCount),
                              detail::Skipper(),
                              commands
                              );

        detail::printComandsList(commands);
        
        std::cout << (r ? "OK" : "FAIL") << std::endl;
        std::cout << "Dogs: " << atMarkCount << std::endl;
        
        if (first != last)
        {
            std::cout << "Can't parse the following: " << std::endl << std::string(first, last) << std::endl;
            return false;
        }
        
        return true;
    }
}

#endif
