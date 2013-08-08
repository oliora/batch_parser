//  Created by Andrey Upadyshev aka Oliora.
//  This code was released to Public domain.

#include "batch_parser.h"
#include <iostream>
#include <fstream>
#include <vector>


namespace
{
    typedef std::vector<char> Buffer;
    
    Buffer readFile(const char *filename)
    {
        std::ifstream is(filename, std::ios::binary);
        
        if (!is)
            throw std::runtime_error("Can not open file");
        
        //is.unsetf(std::ios::skipws);

        is.seekg(0, std::ios_base::end);
        
        Buffer res(is.tellg());

        if (!res.size())
            return Buffer();

        is.seekg(0, std::ios_base::beg);

        // Skip utf-8 BOM if present
        static const char *const UTF8_BOM = "\xEF\xBB\xBF";
        char bom[4] = {};
        if(3 != is.rdbuf()->sgetn(bom, 3) || 0 != strcmp(UTF8_BOM, bom))
        {
            // no BOM, seek back to the begin of file
            is.seekg(0, std::ios_base::beg);
        }
        
        const size_t realSize = is.rdbuf()->sgetn(&res.front(), res.size());
        res.resize(realSize);
        
        return res;
    }
    
    void printCmd(const batch_parser::Command& cmd)
    {
        if (cmd.empty())
        {
            return;
        }
            
        batch_parser::Command::const_iterator first = cmd.begin();
        std::cout << *first << ":" << std::endl;
        ++first;
        
        for(;first != cmd.end(); ++first)
        {
            std::cout << "  " << *first << "," << std::endl;
        }
    }

    void printStatistic(const batch_parser::Statistic& stat)
    {
        std::cout
            << "@: " << stat.m_atMarks << std::endl
            << "IFs: " << stat.m_ifCommands << std::endl
            << "FORs: " << stat.m_forCommands << std::endl
            << "Labels: " << stat.m_labels << std::endl
            << "Comments: " << stat.m_comments << std::endl
            << "Redirects: " << stat.m_redirects << std::endl;
    }
}


int main(int argc, const char * argv[])
{
    try
    {
        if (argc < 2)
        {
            std::cout << "No filename specified" << std::endl;
            return 2;
        }
    
        const Buffer buf(readFile(argv[1]));
        
        //std::cout.write(&buf.front(), buf.size());
    
        batch_parser::Statistic stat;
        
        Buffer::const_iterator first = buf.begin();
        const Buffer::const_iterator last = buf.end();
        
        const bool res = batch_parser::parse(first, last, stat, printCmd);
        
        printStatistic(stat);
        std::cout << (res ? "OK" : "FAIL") << std::endl;
        
        if (first != last)
        {
            std::cerr 
                << "Can't parse the following: " << std::endl
                << std::string(first, last) << std::endl;
            
            return 1;
        }        
        
        return res ? 0 : 1;
    }
    catch (const std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
        return 2;
    }
    
    return 0;
}

