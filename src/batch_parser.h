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
                
        void printLabel(const std::string& label)
        {
            std::cout << ":" << label << std::endl;
        }
                
        typedef boost::spirit::ascii::space_type Skipper;
        
        boost::phoenix::function<Appender> const append;
        boost::phoenix::function<DebugPrinter> const dbg;
        
        template <typename Iterator>
        struct BatchFileGrammar : boost::spirit::qi::grammar<Iterator, Skipper>
        {
            // TODO: move atMarkCount into high level attibute
            template<typename CommandAction, typename LabelAction>
            BatchFileGrammar(size_t& atMarkCount, CommandAction cmdAction, LabelAction labelAction)
                : BatchFileGrammar::base_type(batch)
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
                using boost::spirit::attr;
                using boost::spirit::eol;
                using boost::spirit::eps;
                using boost::spirit::hold;
                using boost::spirit::omit;
                using boost::spirit::raw;
                using boost::spirit::hold;
                using boost::spirit::no_skip;
                using boost::spirit::skip;
                using phoenix::push_back;
                using phoenix::ref;


                arg %= lexeme[
                              +(
                                   (char_ - (  space
                                            | '&'
                                            | '|'
                                            | '\"'
                                            | '^'
                                            | (
                                                  eps(ref(bracketsLevel) != 0)
                                               >> ')' // catch extra closing brackets
                                               )
                                            )
                                    )
                                 | (   lit('^')
                                    >> -(  (omit[eol] >> -((!eol >> char_) | (eol >> attr('\r') >> attr('\n'))))
                                         | (char_ - blank)
                                         )
                                    )
                                 | raw[(   char_('\"')
                                        >> *(char_ - (char_('\"') | eol))
                                        >> -(char_('\"'))
                                        )
                                       ] // I don't know why, but w/o raw[], the rule consumes some unprintable char on EOL/EOF
                                )
                              ];
                
                command %= skip(blank)
                [
                       !char_(':') // to do not capture labels
                    >> +arg
                 ];
                
                label %= skip(blank)
                [
                       omit[   -(char_ - ':')
                            >> ':']
                    >> lexeme[+(char_ - space)]
                    >> omit[*(char_ - eol)]
                 ];
                
                at_mark = char_('@')        [ref(atMarkCount) += 1];
                
                group =
                       char_('(')           [ref(bracketsLevel) += 1]
                    >> *expression
                    >> char_(')')           [ref(bracketsLevel) -= 1]
                    >> -(   eps(ref(bracketsLevel) == 0)
                         >> omit[*char_(')')] // skip all extra closing brackets
                         );
                
                operation =
                    ((  lit("&&")
                      | "||"
                      | '|'
                      | '&'
                      )
                     );

                operand =
                       omit[*at_mark]
                    >> (  group
                        | command           [cmdAction]
                        | label             [labelAction] // TODO: Don't match label if it's not at the begin of line
                        );
                
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
            boost::spirit::qi::rule<Iterator, CommandWithArgs()> command; // It have to has blank_type skipper, but compilation fails in such case. Why?
            boost::spirit::qi::rule<Iterator, std::string()> label; // It have to has blank_type skipper, but compilation fails in such case. Why?
            boost::spirit::qi::rule<Iterator, Skipper> expression;
            boost::spirit::qi::rule<Iterator, Skipper> group;
            boost::spirit::qi::rule<Iterator, Skipper> operand;
            boost::spirit::qi::rule<Iterator, Skipper> at_mark;
            boost::spirit::qi::rule<Iterator, Skipper> operation;
            boost::spirit::qi::rule<Iterator, Skipper> batch;
        };
        
    }
    
    template<typename Iterator>
    inline bool parse(Iterator first, Iterator last)
    {
        namespace qi = boost::spirit::qi;
        namespace ascii = boost::spirit::ascii;
        namespace phoenix = boost::phoenix;
        
        size_t atMarkCount = 0;
        detail::BatchFileGrammar<Iterator> grammar(atMarkCount, detail::printCmd, detail::printLabel);
        
        
        bool r = phrase_parse(
                              first,
                              last,
                              grammar,
                              detail::Skipper()
                              );

        std::cout << "@@@ " << atMarkCount << " @@@" << std::endl;

        std::cout << (r ? "OK" : "FAIL") << std::endl;
        
        if (first != last)
        {
            std::cerr << "Can't parse the following: " << std::endl << std::string(first, last) << std::endl;
            return false;
        }
        
        return true;
    }
}

#endif
