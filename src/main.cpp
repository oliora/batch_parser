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

        if (res.size())
        {
            is.seekg(0, std::ios_base::beg);
            const size_t realSize = is.rdbuf()->sgetn(&res.front(), res.size());
            res.resize(realSize);
        }
        
        return res;
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
    
        const bool res = batch_parser::parse(buf.begin(), buf.end());
        return res ? 0 : 1;
    }
    catch (const std::exception& ex)
    {
        std::cout << "Error: " << ex.what() << std::endl;
        return 2;
    }
    
    return 0;
}

